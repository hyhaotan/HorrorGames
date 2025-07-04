#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyFriendListPopup.generated.h"

class UListView;
class UFriendEntryData;
class ULobbyWidget;

UCLASS()
class HORRORGAME_API ULobbyFriendListPopup : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializePopup(ULobbyWidget* Parent, int32 SlotIndex);

protected:
    UPROPERTY(meta = (BindWidget)) UListView* FriendsListView;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<UFriendEntryData> EntryDataClass;

private:
    ULobbyWidget* ParentLobby = nullptr;
    int32 SlotIndex;

    void PopulateFriendList();
    UFUNCTION() void HandleOnFriendClicked(UObject* Item);
};
