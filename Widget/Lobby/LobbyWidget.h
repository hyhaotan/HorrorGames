#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionClient.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "LobbyWidget.generated.h"

class UPanelWidget;
class ULobbySlotWidget;
class ULobbyFriendListPopup;
class UFriendEntryData;
class UFriendListEntryWidget;

USTRUCT()
struct FLobbySlotData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bOccupied = false;

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	FUniqueNetIdRepl PlayerId;
};

UCLASS()
class HORRORGAME_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	IOnlineFriendsPtr FriendsInterface;

	/** Gửi invite friend cho slot */
	void InviteFriendToSlot(const FUniqueNetIdRepl& FriendId, int32 SlotIndex);

protected:
	/** Steam OSS */
	IOnlineSubsystem* OnlineSubsystem = nullptr;

	IOnlineSessionPtr SessionInterface;
	IOnlineIdentityPtr IdentityInterface;

	/** Container để chứa các widget slot (trong UMG XML) */
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* SlotsContainer;

	/** Class của mỗi slot widget (do Blueprint chỉ định) */
	UPROPERTY(EditAnywhere, Category = "Lobby")
	TSubclassOf<ULobbySlotWidget> LobbySlotWidgetClass;

	/** Class của popup friend list để invite (do Blueprint chỉ định) */
	UPROPERTY(EditAnywhere, Category = "Lobby")
	TSubclassOf<ULobbyFriendListPopup> LobbyFriendListPopupClass;

	/** Handle để xoá popup khi đã dùng */
	UPROPERTY()
	ULobbyFriendListPopup* CurrentFriendListPopup = nullptr;

	/** Danh sách tất cả slot (data) */
	UPROPERTY()
	TArray<FLobbySlotData> LobbySlots;

	/** Danh sách widget instance tương ứng với mỗi slot */
	UPROPERTY()
	TArray<ULobbySlotWidget*> LobbySlotWidgets;

	/** Đăng ký delegate */
	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle StartSessionCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle InviteAcceptedHandle;
	FDelegateHandle LoginCompleteHandle;

	// =============================
	// === Session Key Constants ===
	// =============================

	/** Tên session (static) */
	static const FName SessionName;

	/** Custom key lưu map gameplay */
	static const FName MapNameSettingKey;

	// =============================
	// ===     UFUNCTIONS        ===
	// =============================

	void OnLoginComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	UFUNCTION()
	void OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	UFUNCTION()
	void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);

	UFUNCTION()
	void OnStartSessionComplete(FName InSessionName, bool bWasSuccessful);

	void OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);

	void OnSessionInviteAccepted(bool bWasInvited, int32 LocalPlayerNum, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);

	/** Khi slot “Invite” được click */
	UFUNCTION()
	void HandleOnSlotInviteClicked(int32 SlotIndex);

	/** Tạo session ngay khi vào lobby nếu chưa có */
	void CreatePartySession();

	/** Cập nhật UI của một slot khi trạng thái thay đổi */
	void UpdateSlotWidget(int32 SlotIndex);

	/** Ghi log (wrapper để in kèm tag [Lobby]) */
	void Log(const FString& Msg, ELogVerbosity::Type Verbosity = ELogVerbosity::Log);
};
