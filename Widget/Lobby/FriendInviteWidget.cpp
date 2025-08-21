#include "FriendInviteWidget.h"
#include "HorrorGame/GameInstance/SteamLobbySubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"

void UFriendInviteWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UFriendInviteWidget::OnRefreshClicked);
    }

    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UFriendInviteWidget::OnCloseClicked);
    }

    if (SearchPlayerButton)
    {
        SearchPlayerButton->OnClicked.AddDynamic(this, &UFriendInviteWidget::OnSearchPlayerClicked);
    }

    RefreshFriendsList();
}

void UFriendInviteWidget::RefreshFriendsList()
{
    // Clear current list
    if (FriendsListBox)
    {
        FriendsListBox->ClearChildren();
    }
    CachedFriends.Empty();

    // In a real implementation, you would query Steam friends here
    // For demo purposes, we'll create some mock friends
    TArray<FSteamFriendInfo> MockFriends;

    FSteamFriendInfo Friend1;
    Friend1.SteamID = TEXT("76561198123456789");
    Friend1.DisplayName = TEXT("Player1");
    Friend1.bIsOnline = true;
    Friend1.bIsInGame = false;
    MockFriends.Add(Friend1);

    FSteamFriendInfo Friend2;
    Friend2.SteamID = TEXT("76561198987654321");
    Friend2.DisplayName = TEXT("Player2");
    Friend2.bIsOnline = true;
    Friend2.bIsInGame = true;
    MockFriends.Add(Friend2);

    FSteamFriendInfo Friend3;
    Friend3.SteamID = TEXT("76561198555666777");
    Friend3.DisplayName = TEXT("Player3");
    Friend3.bIsOnline = false;
    Friend3.bIsInGame = false;
    MockFriends.Add(Friend3);

    PopulateFriendsList(MockFriends);
}

void UFriendInviteWidget::SearchPlayerByID(const FString& PlayerID)
{
    if (PlayerID.IsEmpty()) return;

    // Search for player by Player ID
    UE_LOG(LogTemp, Log, TEXT("Searching for player: %s"), *PlayerID);

    // In real implementation, query server for player with this ID
    // For now, just show a message
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Searching for player: %s"), *PlayerID);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, Message);
    }
}

void UFriendInviteWidget::OnRefreshClicked()
{
    RefreshFriendsList();
}

void UFriendInviteWidget::OnCloseClicked()
{
    RemoveFromParent();
}

void UFriendInviteWidget::OnSearchPlayerClicked()
{
    if (PlayerIDSearchBox)
    {
        FString PlayerID = PlayerIDSearchBox->GetText().ToString();
        SearchPlayerByID(PlayerID);
    }
}

void UFriendInviteWidget::PopulateFriendsList(const TArray<FSteamFriendInfo>& Friends)
{
    CachedFriends = Friends;

    for (const FSteamFriendInfo& Friend : Friends)
    {
        AddFriendToList(Friend);
    }
}

void UFriendInviteWidget::AddFriendToList(const FSteamFriendInfo& FriendInfo)
{
    if (!FriendsListBox || !FriendItemWidgetClass) return;

    UUserWidget* FriendItem = CreateWidget<UUserWidget>(this, FriendItemWidgetClass);
    if (IsValid(FriendItem))
    {
        // Set friend data (this would be implemented in the friend item widget)
        FriendsListBox->AddChild(FriendItem);
    }
}