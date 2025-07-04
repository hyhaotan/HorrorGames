#include "LobbyWidget.h"
#include "LobbySlotWidget.h"
#include "LobbyFriendListPopup.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"

void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("Steam"));
    SessionInterface = OSS ? OSS->GetSessionInterface() : nullptr;
    FriendsInterface = OSS ? OSS->GetFriendsInterface() : nullptr;
    IdentityInterface = OSS ? OSS->GetIdentityInterface() : nullptr;

    if (Button_StartGame)
    {
        Button_StartGame->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartClicked);
        // Only host sees
        FNamedOnlineSession* Named = SessionInterface ? SessionInterface->GetNamedSession(NAME_GameSession) : nullptr;
        Button_StartGame->SetVisibility(Named ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    PopulateSlots();
}

void ULobbyWidget::PopulateSlots()
{
    int32 MaxPlayers = 4;
    if (SessionInterface.IsValid())
    {
        FNamedOnlineSession* Named = SessionInterface->GetNamedSession(NAME_GameSession);
        MaxPlayers = Named ? Named->SessionSettings.NumPublicConnections : MaxPlayers;
    }
    SlotDatas.SetNum(MaxPlayers);
    SlotWidgets.SetNum(MaxPlayers);

    FLobbySlotData& Local = SlotDatas[0];
    Local.bOccupied = true;
    if (IdentityInterface.IsValid())
    {
        auto Id = IdentityInterface->GetUniquePlayerId(0);
        Local.PlayerId = FUniqueNetIdRepl(Id.ToSharedRef());
        Local.DisplayName = IdentityInterface->GetPlayerNickname(0);
    }
    else Local.DisplayName = TEXT("Player");

    SlotsContainer->ClearChildren();
    for (int32 i = 0; i < SlotDatas.Num(); ++i)
    {
        FLobbySlotData& D = SlotDatas[i];
        ULobbySlotWidget* W = CreateWidget<ULobbySlotWidget>(GetWorld(), LobbySlotWidgetClass);
        W->InitializeSlot(i, D.bOccupied, D.DisplayName, D.PlayerId);
        W->OnInviteClicked.AddDynamic(this, &ULobbyWidget::HandleSlotInviteClicked);
        SlotWidgets[i] = W;
        SlotsContainer->AddChild(W);
    }
}

void ULobbyWidget::OnStartClicked()
{
    if (!SessionInterface.IsValid()) return;
    SessionInterface->StartSession(NAME_GameSession);
    UGameplayStatics::OpenLevel(this, TEXT("GameplayMap"), true);
}

void ULobbyWidget::HandleSlotInviteClicked(int32 SlotIndex)
{
    if (CurrentPopup) CurrentPopup->RemoveFromParent();
    CurrentPopup = CreateWidget<ULobbyFriendListPopup>(GetWorld(), LobbyFriendListPopupClass);
    CurrentPopup->InitializePopup(this, SlotIndex);
    CurrentPopup->AddToViewport();
}

void ULobbyWidget::InviteFriendToSlot(const FUniqueNetIdRepl& FriendId, int32 SlotIndex)
{
    if (!SessionInterface.IsValid() || !SlotDatas.IsValidIndex(SlotIndex)) return;
    SessionInterface->SendSessionInviteToFriend(0, NAME_GameSession, *FriendId);
    FLobbySlotData& D = SlotDatas[SlotIndex];
    D.bOccupied = true;
    D.PlayerId = FriendId;
    D.DisplayName = TEXT("Invited");
    UpdateSlot(SlotIndex);
    if (CurrentPopup) CurrentPopup->RemoveFromParent();
}

void ULobbyWidget::UpdateSlot(int32 SlotIndex)
{
    if (!SlotWidgets.IsValidIndex(SlotIndex)) return;
    FLobbySlotData& D = SlotDatas[SlotIndex];
    SlotWidgets[SlotIndex]->InitializeSlot(SlotIndex, D.bOccupied, D.DisplayName, D.PlayerId);
}