#include "LobbyFriendListPopup.h"
#include "HorrorGame/Object/FriendEntryData.h"
#include "Components/ListView.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "LobbyWidget.h"

void ULobbyFriendListPopup::InitializePopup(ULobbyWidget* Parent, int32 InSlotIndex)
{
    ParentLobby = Parent;
    SlotIndex = InSlotIndex;
    if (FriendsListView)
    {
        FriendsListView->ClearListItems();
        FriendsListView->OnItemClicked().AddUObject(this, &ULobbyFriendListPopup::HandleOnFriendClicked);
        PopulateFriendList();
    }
}

void ULobbyFriendListPopup::PopulateFriendList()
{
    if (!ParentLobby || !ParentLobby->FriendsInterface) return;
    FriendsListView->ClearListItems();
    TArray<TSharedRef<FOnlineFriend>> Friends;
    ParentLobby->FriendsInterface->GetFriendsList(0, TEXT("default"), Friends);
    for (auto& FriendRef : Friends)
    {
        UFriendEntryData* Entry = NewObject<UFriendEntryData>(this, EntryDataClass);
        Entry->DisplayName = FriendRef->GetDisplayName();
        Entry->UniqueIdRepl = FUniqueNetIdRepl(FriendRef->GetUserId());
        FriendsListView->AddItem(Entry);
    }
}

void ULobbyFriendListPopup::HandleOnFriendClicked(UObject* Item)
{
    if (!Item || !ParentLobby) return;
    UFriendEntryData* Data = Cast<UFriendEntryData>(Item);
    if (!Data) return;
    ParentLobby->InviteFriendToSlot(Data->UniqueIdRepl, SlotIndex);
    RemoveFromParent();
}