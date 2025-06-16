#include "LobbyFriendListPopup.h"
#include "HorrorGame/Widget/Lobby/LobbyWidget.h"
#include "HorrorGame/Object/FriendEntryData.h"
#include "HorrorGame/Widget/Lobby/FriendListEntryWidget.h"
#include "OnlineSubsystemUtils.h"
#include "Components/ListView.h"

void ULobbyFriendListPopup::InitializePopup(ULobbyWidget* ParentLobbyWidget, int32 InSlotIndex)
{
    ParentLobbyWidgetRef = ParentLobbyWidget;
    SlotIndex = InSlotIndex;

    // Thiết lập ListView: class EntryData, widget cho mỗi entry
    if (FriendsListView)
    {
        FriendsListView->GetEntryWidgetClass();
        FriendsListView->SetListItems(TArray<UObject*>()); // clear
        FriendsListView->OnItemClicked().AddUObject(this, &ULobbyFriendListPopup::HandleOnFriendClicked);

        // Populate dữ liệu friend (dựa trên ParentLobbyWidget đã ReadFriendsList)
        PopulateFriendList();
    }
}

void ULobbyFriendListPopup::PopulateFriendList()
{
    if (!ParentLobbyWidgetRef || !ParentLobbyWidgetRef->FriendsInterface.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("PopulateFriendList: FriendsInterface invalid"));
        return;
    }

    // Trước hết clear lại UI
    FriendsListView->ClearListItems();

    // Lấy danh sách friend từ Steam (backend đã load ở OnReadFriendsListComplete)
    TArray<TSharedRef<FOnlineFriend>> Friends;
    ParentLobbyWidgetRef->FriendsInterface->GetFriendsList(0, TEXT("default"), Friends);

    UE_LOG(LogTemp, Log, TEXT("PopulateFriendList: Lấy được %d friends"), Friends.Num());

    for (const auto& FriendRef : Friends)
    {
        UFriendEntryData* Entry = NewObject<UFriendEntryData>(this, FriendEntryDataClass);
        Entry->DisplayName = FriendRef->GetDisplayName();
        Entry->UniqueIdRepl = FUniqueNetIdRepl(FriendRef->GetUserId());
        FriendsListView->AddItem(Entry);
        UE_LOG(LogTemp, Log, TEXT("  + Thêm friend: %s"), *Entry->DisplayName);
    }
}

void ULobbyFriendListPopup::HandleOnFriendClicked(UObject* ClickedItem)
{
    if (!ClickedItem || !ParentLobbyWidgetRef)
    {
        return;
    }

    UFriendEntryData* Data = Cast<UFriendEntryData>(ClickedItem);
    if (!Data)
    {
        return;
    }

    // Tell the parent to send a Steam invite for this friend + slot
    ParentLobbyWidgetRef->InviteFriendToSlot(Data->UniqueIdRepl, SlotIndex);

    // After inviting, just close this popup
    RemoveFromParent();
}
