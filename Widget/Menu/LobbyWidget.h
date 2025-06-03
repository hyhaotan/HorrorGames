#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "LobbyWidget.generated.h"

class ULobbySlotWidget;
class ULobbyFriendListPopup;

/**
 * Data structure for one lobby slot.
 */
USTRUCT(BlueprintType)
struct FLobbySlotData
{
	GENERATED_BODY()

	/** Whether a player is occupying this slot (has joined or is local). */
	UPROPERTY()
	bool bOccupied = false;

	/** If occupied or invited, we store the player’s unique ID here. */
	UPROPERTY()
	FUniqueNetIdRepl PlayerId;

	/** The display name to show (either local player or “Invited...” or friend’s name). */
	UPROPERTY()
	FString DisplayName;
};

/**
 * The main lobby widget.  On construct, logs into Steam, reads friends,
 * creates a session (for hosting), and sets up 4 slot widgets.
 */
UCLASS()
class HORRORGAME_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Called from ULobbyFriendListPopup when the user picks a friend to invite. */
	void InviteFriendToSlot(const FUniqueNetIdRepl& FriendId, int32 SlotIndex);

	/** Logs to Output Log + optionally to screen. */
	void Log(const FString& Msg, ELogVerbosity::Type Verbosity = ELogVerbosity::Log);

	IOnlineFriendsPtr FriendsInterface;

	static const FName MapNameSettingKey;

protected:
	/** Bound to a Panel (e.g. VerticalBox or UniformGrid) in your UMG. */
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* SlotsContainer;

	/** The LobbySlot widget blueprint class (derived from ULobbySlotWidget). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<ULobbySlotWidget> LobbySlotWidgetClass;

	/** The FriendListPopup blueprint class (derived from ULobbyFriendListPopup). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<ULobbyFriendListPopup> LobbyFriendListPopupClass;

private:
	/** An array of slot data (size = 4). */
	UPROPERTY()
	TArray<FLobbySlotData> LobbySlots;

	/** The actual widget objects for each slot (size = 4). */
	UPROPERTY()
	TArray<ULobbySlotWidget*> LobbySlotWidgets;

	/** If a friend‐list popup is currently up, we keep a reference to remove it later. */
	UPROPERTY()
	ULobbyFriendListPopup* CurrentFriendListPopup = nullptr;

	//– OnlineSubsystem pointers –––
	IOnlineSubsystem* OnlineSubsystem = nullptr;
	IOnlineSessionPtr SessionInterface;
	IOnlineIdentityPtr IdentityInterface;

	// Delegate handles so we can unbind if needed
	FDelegateHandle LoginCompleteHandle;
	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle StartSessionCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle InviteAcceptedHandle;

	// The session name we’ll use for Create/Join
	static const FName SessionName;
	// The key we’ll store our “real game map” under so that StartSession knows which map to load

private:
	// Called when Steam login completes
	void OnLoginComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	// Called when Steam has read the friend list
	void OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	// Host: create a party session
	void CreatePartySession();

	// Called when session creation finishes (host)
	void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);

	// Called when StartSession finishes (host)
	void OnStartSessionComplete(FName InSessionName, bool bWasSuccessful);

	// Called when JoinSession finishes (client)
	void OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);

	// Called when this client accepts an invite from a friend
	void OnSessionInviteAccepted(bool bWasInvited, int32 LocalPlayerNum, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);

	// Called when one of our SlotWidgets broadcasts OnInviteClicked
	UFUNCTION()
	void HandleOnSlotInviteClicked(int32 SlotIndex);

	// Refresh one slot’s visual based on LobbySlots[SlotIndex]
	void UpdateSlotWidget(int32 SlotIndex);
};
