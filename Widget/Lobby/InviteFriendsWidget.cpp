#include "HorrorGame/Widget/Lobby/InviteFriendsWidget.h"
#include "Components/Button.h"
#include "Engine/Engine.h"

void UInviteFriendsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (InviteButton)
    {
        InviteButton->OnClicked.AddDynamic(this, &UInviteFriendsWidget::HandleInviteClicked);
    }

    if (LeaveButton)
    {
        LeaveButton->OnClicked.AddDynamic(this, &UInviteFriendsWidget::HandleLeaveClicked);
    }

    UpdateButtonVisibility();
}

void UInviteFriendsWidget::SetOccupied(bool bOccupied)
{
    bIsOccupied = bOccupied;
    UpdateButtonVisibility();
}

void UInviteFriendsWidget::HandleInviteClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Invite button clicked for slot %d"), SlotIndex);
    OnInviteClicked.Broadcast(SlotIndex);
}

void UInviteFriendsWidget::HandleLeaveClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Leave button clicked for slot %d"), SlotIndex);
    OnLeaveClicked.Broadcast(SlotIndex);
}

void UInviteFriendsWidget::UpdateButtonVisibility()
{
    if (InviteButton)
    {
        InviteButton->SetVisibility(bIsOccupied ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }

    if (LeaveButton)
    {
        LeaveButton->SetVisibility(bIsOccupied ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}