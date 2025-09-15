#include "NotificationWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNotificationWidget::UNotificationWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsShowing(false)
    , CurrentType(ENotificationType::Info)
{
    // Initialize default styles
    InfoStyle.BackgroundColor = FLinearColor(0.2f, 0.4f, 0.8f, 0.9f);
    InfoStyle.TextColor = FLinearColor::White;
    InfoStyle.IconColor = FLinearColor::White;
    InfoStyle.DisplayDuration = 3.0f;

    SuccessStyle.BackgroundColor = FLinearColor(0.2f, 0.8f, 0.3f, 0.9f);
    SuccessStyle.TextColor = FLinearColor::White;
    SuccessStyle.IconColor = FLinearColor::White;
    SuccessStyle.DisplayDuration = 2.5f;

    WarningStyle.BackgroundColor = FLinearColor(0.9f, 0.7f, 0.2f, 0.9f);
    WarningStyle.TextColor = FLinearColor::Black;
    WarningStyle.IconColor = FLinearColor::Black;
    WarningStyle.DisplayDuration = 4.0f;

    ErrorStyle.BackgroundColor = FLinearColor(0.8f, 0.2f, 0.2f, 0.9f);
    ErrorStyle.TextColor = FLinearColor::White;
    ErrorStyle.IconColor = FLinearColor::White;
    ErrorStyle.DisplayDuration = 5.0f;
}

void UNotificationWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initially hide the notification
    SetVisibility(ESlateVisibility::Collapsed);

    // Bind animation events
    if (SlideInAnimation)
    {
        FWidgetAnimationDynamicEvent SlideInFinished;
        SlideInFinished.BindDynamic(this, &UNotificationWidget::OnSlideInFinished);
        BindToAnimationFinished(SlideInAnimation, SlideInFinished);
    }

    if (SlideOutAnimation)
    {
        FWidgetAnimationDynamicEvent SlideOutFinished;
        SlideOutFinished.BindDynamic(this, &UNotificationWidget::OnSlideOutFinished);
        BindToAnimationFinished(SlideOutAnimation, SlideOutFinished);
    }
}

void UNotificationWidget::ShowNotification(const FString& Message, ENotificationType Type)
{
    if (bIsShowing)
    {
        // Queue this notification if one is already showing
        QueueNotification(Message, Type);
        return;
    }

    bIsShowing = true;
    CurrentType = Type;

    // Set message text
    if (MessageText)
    {
        MessageText->SetText(FText::FromString(Message));
    }

    // Apply visual style
    ApplyNotificationStyle(Type);

    // Play sound
    PlayNotificationSound(Type);

    // Show the widget
    SetVisibility(ESlateVisibility::Visible);

    // Play slide-in animation
    if (SlideInAnimation)
    {
        PlayAnimation(SlideInAnimation);
    }
    else
    {
        OnSlideInFinished();
    }

    // Call Blueprint event
    OnNotificationShown(Message, Type);
}

void UNotificationWidget::HideNotification()
{
    if (!bIsShowing)
        return;

    // Clear hide timer
    GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);

    // Play slide-out animation
    if (SlideOutAnimation)
    {
        PlayAnimation(SlideOutAnimation);
    }
    else
    {
        OnSlideOutFinished();
    }
}

void UNotificationWidget::QueueNotification(const FString& Message, ENotificationType Type)
{
    NotificationQueue.Add(Message);
    TypeQueue.Add(Type);
}

void UNotificationWidget::ClearNotificationQueue()
{
    NotificationQueue.Empty();
    TypeQueue.Empty();
}

void UNotificationWidget::ApplyNotificationStyle(ENotificationType Type)
{
    FNotificationStyle Style = GetNotificationStyle(Type);

    // Apply background color
    if (NotificationBorder)
    {
        NotificationBorder->SetBrushColor(Style.BackgroundColor);
    }

    // Apply text color
    if (MessageText)
    {
        MessageText->SetColorAndOpacity(Style.TextColor);
    }

    // Apply icon
    if (NotificationIcon)
    {
        if (Style.IconTexture)
        {
            NotificationIcon->SetBrushFromTexture(Style.IconTexture);
            NotificationIcon->SetColorAndOpacity(Style.IconColor);
            NotificationIcon->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            NotificationIcon->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UNotificationWidget::PlayNotificationSound(ENotificationType Type)
{
    USoundCue* SoundToPlay = nullptr;

    switch (Type)
    {
    case ENotificationType::Info:
        SoundToPlay = InfoSound;
        break;
    case ENotificationType::Success:
        SoundToPlay = SuccessSound;
        break;
    case ENotificationType::Warning:
        SoundToPlay = WarningSound;
        break;
    case ENotificationType::Error:
        SoundToPlay = ErrorSound;
        break;
    }

    if (SoundToPlay)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), SoundToPlay);
    }
}

void UNotificationWidget::ProcessNotificationQueue()
{
    if (NotificationQueue.Num() > 0 && TypeQueue.Num() > 0)
    {
        FString NextMessage = NotificationQueue[0];
        ENotificationType NextType = TypeQueue[0];

        NotificationQueue.RemoveAt(0);
        TypeQueue.RemoveAt(0);

        ShowNotification(NextMessage, NextType);
    }
}

void UNotificationWidget::OnSlideInFinished()
{
    // Start auto-hide timer
    FNotificationStyle Style = GetNotificationStyle(CurrentType);

    GetWorld()->GetTimerManager().SetTimer(HideTimerHandle, [this]()
        {
            HideNotification();
        }, Style.DisplayDuration, false);

    // Play pulse animation for attention
    if (PulseAnimation && (CurrentType == ENotificationType::Warning || CurrentType == ENotificationType::Error))
    {
        PlayAnimation(PulseAnimation, 0.0f, 2); // Play twice
    }
}

void UNotificationWidget::OnSlideOutFinished()
{
    bIsShowing = false;
    SetVisibility(ESlateVisibility::Collapsed);

    // Call Blueprint event
    OnNotificationHidden();

    // Process queue
    ProcessNotificationQueue();
}

void UNotificationWidget::SetNotificationStyle(ENotificationType Type, const FNotificationStyle& Style)
{
    switch (Type)
    {
    case ENotificationType::Info:
        InfoStyle = Style;
        break;
    case ENotificationType::Success:
        SuccessStyle = Style;
        break;
    case ENotificationType::Warning:
        WarningStyle = Style;
        break;
    case ENotificationType::Error:
        ErrorStyle = Style;
        break;
    }
}

FNotificationStyle UNotificationWidget::GetNotificationStyle(ENotificationType Type) const
{
    switch (Type)
    {
    case ENotificationType::Info:
        return InfoStyle;
    case ENotificationType::Success:
        return SuccessStyle;
    case ENotificationType::Warning:
        return WarningStyle;
    case ENotificationType::Error:
        return ErrorStyle;
    default:
        return InfoStyle;
    }
}