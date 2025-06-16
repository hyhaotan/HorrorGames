#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HorrorGame/Object/FriendEntryData.h"
#include "FriendListEntryWidget.generated.h"

class UTextBlock;
class UButton;

DECLARE_DELEGATE_OneParam(FOnInviteClicked, UFriendEntryData* /*FriendData*/);

UCLASS()
class HORRORGAME_API UFriendListEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Gọi từ LobbyWidget để khởi tạo dữ liệu
    void Setup(UFriendEntryData* InData, FOnInviteClicked InDelegate);

protected:
    virtual void NativeConstruct() override;

    // Widget bindings
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_FriendName;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Invite;

private:
    UFriendEntryData* FriendData;
    FOnInviteClicked InviteDelegate;

    UFUNCTION()
    void OnInviteButtonPressed();
};
