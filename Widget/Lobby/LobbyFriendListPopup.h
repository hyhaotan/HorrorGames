#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystemTypes.h"
#include "LobbyFriendListPopup.generated.h"

class UListView;
class UFriendEntryData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFriendSelected, const FUniqueNetIdRepl&, FriendId, int32, SlotIndex);

/**
 * ULobbyFriendListPopup:
 *  - Hiển thị một list view chứa danh sách bạn bè (Read từ Steam).
 *  - Khi người chơi chọn friend, gọi OnFriendSelected.Broadcast(FriendId, SlotIndex).
 */
UCLASS()
class HORRORGAME_API ULobbyFriendListPopup : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Gọi để initialize popup, truyền vào widget cha và slotIndex để callback */
    UFUNCTION(BlueprintCallable, Category = "FriendPopup")
    void InitializePopup(class ULobbyWidget* ParentLobbyWidget, int32 InSlotIndex);

protected:
    // --- BindWidget từ UMG Blueprint ---
    UPROPERTY(meta = (BindWidget))
    UListView* FriendsListView;

    // Class cho entry Data (dùng FriendEntryData)
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UFriendEntryData> FriendEntryDataClass;

    // Class cho widget hiển thị mỗi entry
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UFriendListEntryWidget> FriendListEntryWidgetClass;

    // SlotIndex truyền vào từ parent
    int32 SlotIndex;

    // Reference tới widget cha (LobbyWidget) để callback
    UPROPERTY()
    ULobbyWidget* ParentLobbyWidgetRef;

    // Callback khi chọn một item (friend) trong list view
    UFUNCTION()
    void HandleOnFriendClicked(UObject* ClickedItem);
    // (Bạn có thể customize để truyền FUniqueNetIdRepl, không nhất thiết truyền string)

    // Sau khi ReadFriendsListComplete (ở parent đã handle), data đã sẵn sàng
    // Ta cần populate FriendsListView bằng các UFriendEntryData
    void PopulateFriendList();
};
