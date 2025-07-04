#include "SessionRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void USessionRowWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &USessionRowWidget::OnJoinButtonClicked);
    }
}

void USessionRowWidget::Setup(const FSessionSettingsData& InData, int32 InIndex)
{
    RowIndex = InIndex;
    if (RoomNameText)
        RoomNameText->SetText(FText::FromString(InData.RoomName));
    if (MapNameText)
        MapNameText->SetText(FText::FromString(InData.MapName));
    if (PlayerCountText)
    {
        // Hiển thị max players; current players không có dữ liệu cụ thể
        FString CountStr = FString::Printf(TEXT("? / %d"), InData.MaxPlayers);
        PlayerCountText->SetText(FText::FromString(CountStr));
    }
}

void USessionRowWidget::OnJoinButtonClicked()
{
    OnJoinSessionClicked.Broadcast(RowIndex);
}
