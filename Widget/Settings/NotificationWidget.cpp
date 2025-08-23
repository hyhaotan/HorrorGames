#include "NotificationWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UNotificationWidget::ShowNotification(const FText& Message, float Duration)
{
    if (NotificationText)
    {
        NotificationText->SetText(Message);
    }

    // Clear any existing timer
    if (GetWorld() && HideTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(HideTimer);
    }

    // Show the widget
    SetVisibility(ESlateVisibility::Visible);

    // Play fade in animation
    if (FadeInOut)
    {
        PlayAnimation(FadeInOut);
    }

    // Set timer to hide after duration
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(HideTimer, this, &UNotificationWidget::HideNotification, Duration, false);
    }
}

void UNotificationWidget::HideNotification()
{
    // Play fade out animation or just hide
    SetVisibility(ESlateVisibility::Hidden);
}