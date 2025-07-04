#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "LobbyWidget.generated.h"

class UPanelWidget;
class ULobbySlotWidget;
class ULobbyFriendListPopup;
class UButton;

USTRUCT()
struct FLobbySlotData
{
    GENERATED_BODY()
    bool bOccupied = false;
    FString DisplayName;
    FUniqueNetIdRepl PlayerId;
};

UCLASS()
class HORRORGAME_API ULobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    IOnlineFriendsPtr FriendsInterface;

protected:
    UPROPERTY(meta = (BindWidget)) UPanelWidget* SlotsContainer;
    UPROPERTY(meta = (BindWidget)) UButton* Button_StartGame;

    UPROPERTY(EditAnywhere, Category = "Lobby") TSubclassOf<ULobbySlotWidget> LobbySlotWidgetClass;
    UPROPERTY(EditAnywhere, Category = "Lobby") TSubclassOf<ULobbyFriendListPopup> LobbyFriendListPopupClass;

private:
    IOnlineSessionPtr SessionInterface;
    IOnlineIdentityPtr IdentityInterface;

    TArray<FLobbySlotData> SlotDatas;
    TArray<ULobbySlotWidget*> SlotWidgets;
    ULobbyFriendListPopup* CurrentPopup = nullptr;

    void PopulateSlots();
    UFUNCTION() void OnStartClicked();
    UFUNCTION() void HandleSlotInviteClicked(int32 SlotIndex);
    void UpdateSlot(int32 SlotIndex);

public:
    // Called by popup
    void InviteFriendToSlot(const FUniqueNetIdRepl& FriendId, int32 SlotIndex);
};