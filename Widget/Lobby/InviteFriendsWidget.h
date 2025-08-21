#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "InviteFriendsWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteClicked, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeaveClicked, int32, SlotIndex);

UCLASS()
class HORRORGAME_API UInviteFriendsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // Thiết lập trạng thái slot (occupied hay không)
    UFUNCTION(BlueprintCallable, Category = "Invite")
    void SetOccupied(bool bOccupied);

    // Slot index tương ứng (do parent gán)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Invite")
    int32 SlotIndex = -1;

    // Sự kiện để parent bind (Blueprint/Code)
    UPROPERTY(BlueprintAssignable, Category = "Invite")
    FOnInviteClicked OnInviteClicked;

    UPROPERTY(BlueprintAssignable, Category = "Invite")
    FOnLeaveClicked OnLeaveClicked;

protected:
    // Buttons (bind từ UMG)
    UPROPERTY(meta = (BindWidget))
    UButton* InviteButton;

    UPROPERTY(meta = (BindWidget))
    UButton* LeaveButton;

    // Trạng thái runtime
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Invite")
    bool bIsOccupied = false;

    UFUNCTION()
    void HandleInviteClicked();

    UFUNCTION()
    void HandleLeaveClicked();

    // Cập nhật visibility của nút dựa trên trạng thái
    void UpdateButtonVisibility();
};