#include "KeyNotificationWidget.h"
#include "Components/TextBlock.h"

void UKeyNotificationWidget::UpdateKeyNotification(const FString& KeyName)
{
    if (!KeyNotificationText)
        return;

    // Ví dụ: "Bạn cần chìa khóa BedRoomKey để mở cửa này"
    FString Message = FString::Printf(TEXT("Bạn cần %s để mở cửa này"), *KeyName);
    KeyNotificationText->SetText(FText::FromString(Message));

    // Show widget
    this->SetVisibility(ESlateVisibility::Visible);

	FTimerHandle HideTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(HideTimerHandle, this, &UKeyNotificationWidget::SetDisplayText, 3.0f, false);
}

void UKeyNotificationWidget::SetDisplayText()
{
    this->SetVisibility(ESlateVisibility::Hidden);
}
