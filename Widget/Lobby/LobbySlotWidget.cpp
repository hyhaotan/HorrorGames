#include "LobbySlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"

void ULobbySlotWidget::InitializeSlot(int32 InSlotIndex, bool bIsOccupied, const FString& DisplayName, const FUniqueNetIdRepl& InUniqueId)
{
    SlotIndex = InSlotIndex;
    bOccupied = bIsOccupied;
    UniqueId = InUniqueId;

    if (bOccupied)
    {
        // Nếu đã có người: show tên + avatar, ẩn nút Invite
        if (InviteButton)
        {
            InviteButton->SetVisibility(ESlateVisibility::Collapsed);
        }
        if (PlayerNameText)
        {
            PlayerNameText->SetVisibility(ESlateVisibility::Visible);
            PlayerNameText->SetText(FText::FromString(DisplayName));
        }
        if (PlayerAvatarImage)
        {
            PlayerAvatarImage->SetVisibility(ESlateVisibility::Visible);
            // Có thể set brush/sprite cho avatar tại đây nếu cần
        }
    }
    else
    {
        // Nếu trống: ẩn tên + avatar, show Invite button
        if (PlayerNameText)
        {
            PlayerNameText->SetVisibility(ESlateVisibility::Collapsed);
        }
        if (PlayerAvatarImage)
        {
            PlayerAvatarImage->SetVisibility(ESlateVisibility::Collapsed);
        }
        if (InviteButton)
        {
            InviteButton->SetVisibility(ESlateVisibility::Visible);
            InviteButton->OnClicked.Clear();
            InviteButton->OnClicked.AddDynamic(this, &ULobbySlotWidget::HandleInviteButtonClicked);
        }
    }
}

void ULobbySlotWidget::HandleInviteButtonClicked()
{
    if (!bOccupied)
    {
        OnInviteClicked.Broadcast(SlotIndex);
    }
}
