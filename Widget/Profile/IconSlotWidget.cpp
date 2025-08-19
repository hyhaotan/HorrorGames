// Fill out your copyright notice in the Description page of Project Settings.

#include "IconSlotWidget.h"
#include "IconSlotContentWidget.h"
#include "PlayerProfileHUDWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"

void UIconSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Setup image hover events
    SetupImageHoverEvents();

    // Initialize border colors
    InitializeBorderColors();
}

void UIconSlotWidget::NativeDestruct()
{
    // Hide content widget if showing
    HideContentWidget();

    // Clear animation timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(IconBorderFirstTimerHandle);
        World->GetTimerManager().ClearTimer(IconBorderSecondTimerHandle);
        World->GetTimerManager().ClearTimer(IconBorderThirdTimerHandle);
    }

    Super::NativeDestruct();
}

void UIconSlotWidget::SetupImageHoverEvents()
{
    // Just ensure images are visible - no event binding needed
    if (IconImageFirst)
    {
        IconImageFirst->SetVisibility(ESlateVisibility::Visible);
    }

    if (IconImageSecond)
    {
        IconImageSecond->SetVisibility(ESlateVisibility::Visible);
    }

    if (IconImageThird)
    {
        IconImageThird->SetVisibility(ESlateVisibility::Visible);
    }
}

void UIconSlotWidget::InitializeBorderColors()
{
    if (IconBorderFirst)
    {
        IconBorderFirst->SetBrushColor(NormalBorderColor);
    }

    if (IconBorderSecond)
    {
        IconBorderSecond->SetBrushColor(NormalBorderColor);
    }

    if (IconBorderThird)
    {
        IconBorderThird->SetBrushColor(NormalBorderColor);
    }
}

int32 UIconSlotWidget::GetHoveredImageIndex(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!IconImageFirst || !IconImageSecond || !IconImageThird)
        return -1;

    // Check if mouse is over first image
    if (IconImageFirst->GetVisibility() == ESlateVisibility::Visible)
    {
        FGeometry ImageGeometry = IconImageFirst->GetCachedGeometry();
        if (ImageGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
        {
            return 0;
        }
    }

    // Check if mouse is over second image
    if (IconImageSecond->GetVisibility() == ESlateVisibility::Visible)
    {
        FGeometry ImageGeometry = IconImageSecond->GetCachedGeometry();
        if (ImageGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
        {
            return 1;
        }
    }

    // Check if mouse is over third image
    if (IconImageThird->GetVisibility() == ESlateVisibility::Visible)
    {
        FGeometry ImageGeometry = IconImageThird->GetCachedGeometry();
        if (ImageGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
        {
            return 2;
        }
    }

    return -1; // No image hovered
}

int32 UIconSlotWidget::GetClickedImageIndex(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // Reuse the hover detection logic for click detection
    return GetHoveredImageIndex(InGeometry, InMouseEvent);
}

void UIconSlotWidget::HandleImageHover(int32 ImageIndex, bool bIsHovered)
{
    switch (ImageIndex)
    {
    case 0: // First image
        if (bIsHovered)
            OnIconImageFirstHovered();
        else
            OnIconImageFirstUnhovered();
        break;

    case 1: // Second image
        if (bIsHovered)
            OnIconImageSecondHovered();
        else
            OnIconImageSecondUnhovered();
        break;

    case 2: // Third image
        if (bIsHovered)
            OnIconImageThirdHovered();
        else
            OnIconImageThirdUnhovered();
        break;

    case -1: // No image hovered, reset all
        OnIconImageFirstUnhovered();
        OnIconImageSecondUnhovered();
        OnIconImageThirdUnhovered();
        break;
    }
}

void UIconSlotWidget::HandleImageClick(int32 ImageIndex)
{
    if (ImageIndex >= 0 && ImageIndex <= 2)
    {
        UE_LOG(LogTemp, Log, TEXT("IconSlotWidget: Image %d clicked"), ImageIndex);

        // If clicking the same image that's already selected, hide the content
        if (CurrentSelectedImageIndex == ImageIndex && CurrentContentWidget)
        {
            HideContentWidget();
        }
        else
        {
            // Show content for the clicked image
            ShowContentWidget(ImageIndex);
        }

        // Broadcast the click event
        OnIconClicked.Broadcast(ImageIndex, this);
    }
}

void UIconSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    // Check which image is being hovered and animate accordingly
    int32 HoveredIndex = GetHoveredImageIndex(InGeometry, InMouseEvent);
    HandleImageHover(HoveredIndex, true);
}

void UIconSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    // Reset all borders to normal when leaving the widget
    HandleImageHover(-1, false);
}

FReply UIconSlotWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseMove(InGeometry, InMouseEvent);

    static int32 LastHoveredIndex = -1;
    int32 CurrentHoveredIndex = GetHoveredImageIndex(InGeometry, InMouseEvent);

    if (CurrentHoveredIndex != LastHoveredIndex)
    {
        // Unhover previous image
        if (LastHoveredIndex >= 0)
        {
            HandleImageHover(LastHoveredIndex, false);
        }

        // Hover new image
        if (CurrentHoveredIndex >= 0)
        {
            HandleImageHover(CurrentHoveredIndex, true);
        }

        LastHoveredIndex = CurrentHoveredIndex;
    }
    return FReply::Handled();
}

FReply UIconSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

    // Only handle left mouse button clicks
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        int32 ClickedIndex = GetClickedImageIndex(InGeometry, InMouseEvent);
        HandleImageClick(ClickedIndex);
        return FReply::Handled();
    }

    return FReply::Unhandled();
}

void UIconSlotWidget::ShowContentWidget(int32 ImageIndex)
{
    if (ImageIndex < 0 || ImageIndex > 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("IconSlotWidget: Invalid image index %d"), ImageIndex);
        return;
    }

    if (!IconSlotContentWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("IconSlotWidget: IconSlotContentWidgetClass is not set"));
        return;
    }

    // Hide existing content widget if any
    HideContentWidget();

    // Create new content widget
    CurrentContentWidget = CreateWidget<UIconSlotContentWidget>(GetWorld(), IconSlotContentWidgetClass);
    if (!CurrentContentWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("IconSlotWidget: Failed to create IconSlotContentWidget"));
        return;
    }

    // Add to viewport
    CurrentContentWidget->AddToViewport(100); // High Z-order to appear on top

    // Position the content widget near the clicked image
    PositionContentWidget(ImageIndex);

    // Store the currently selected image index
    CurrentSelectedImageIndex = ImageIndex;

    UE_LOG(LogTemp, Log, TEXT("IconSlotWidget: Content widget shown for image %d"), ImageIndex);
}

void UIconSlotWidget::HideContentWidget()
{
    if (CurrentContentWidget)
    {
        CurrentContentWidget->RemoveFromParent();
        CurrentContentWidget = nullptr;
        CurrentSelectedImageIndex = -1;

        UE_LOG(LogTemp, Log, TEXT("IconSlotWidget: Content widget hidden"));
    }
}

void UIconSlotWidget::PositionContentWidget(int32 ImageIndex)
{
    if (!CurrentContentWidget)
        return;

    UImage* TargetImage = nullptr;
    switch (ImageIndex)
    {
    case 0:
        TargetImage = IconImageFirst;
        break;
    case 1:
        TargetImage = IconImageSecond;
        break;
    case 2:
        TargetImage = IconImageThird;
        break;
    default:
        return;
    }

    if (!TargetImage)
        return;

    // Get the image's screen position and size
    FGeometry ImageGeometry = TargetImage->GetCachedGeometry();
    FVector2D ImageScreenPosition = ImageGeometry.GetAbsolutePosition();
    FVector2D ImageSize = ImageGeometry.GetAbsoluteSize();

    // Get viewport size for boundary checking
    FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());

    // Calculate desired position (to the right of the image with some offset)
    FVector2D DesiredPosition = ImageScreenPosition + FVector2D(ImageSize.X + 20.0f, 0.0f);

    // Get content widget size (assume a default size if not yet rendered)
    FVector2D ContentSize = FVector2D(300.0f, 200.0f); // Default size, adjust as needed

    // Boundary checking - if content would go off screen, position it to the left instead
    if (DesiredPosition.X + ContentSize.X > ViewportSize.X)
    {
        DesiredPosition.X = ImageScreenPosition.X - ContentSize.X - 20.0f;
    }

    // Ensure content doesn't go off the top or bottom of screen
    if (DesiredPosition.Y + ContentSize.Y > ViewportSize.Y)
    {
        DesiredPosition.Y = ViewportSize.Y - ContentSize.Y - 20.0f;
    }
    if (DesiredPosition.Y < 0)
    {
        DesiredPosition.Y = 20.0f;
    }

    // Set the position
    CurrentContentWidget->SetPositionInViewport(DesiredPosition, false);
}

void UIconSlotWidget::OnIconImageFirstHovered()
{
    UE_LOG(LogTemp, Log, TEXT("First icon image hovered"));
    if (IconBorderFirst)
    {
        bIsSlotHover = true;
        AnimateBorderToHover(IconBorderFirst, IconBorderFirstTimerHandle, 0.2f);
        OnSlotHover.Broadcast(bIsSlotHover);
    }
}

void UIconSlotWidget::OnIconImageFirstUnhovered()
{
    UE_LOG(LogTemp, Log, TEXT("First icon image unhovered"));
    if (IconBorderFirst)
    {
        bIsSlotHover = false;
        AnimateBorderToNormal(IconBorderFirst, IconBorderFirstTimerHandle, 0.3f);
        OnSlotHover.Broadcast(bIsSlotHover);
    }
}

void UIconSlotWidget::OnIconImageSecondHovered()
{
    UE_LOG(LogTemp, Log, TEXT("Second icon image hovered"));
    if (IconBorderSecond)
    {
        bIsSlotHover = true;
        AnimateBorderToHover(IconBorderSecond, IconBorderSecondTimerHandle, 0.2f);
        OnSlotHover.Broadcast(bIsSlotHover);
    }
}

void UIconSlotWidget::OnIconImageSecondUnhovered()
{
    UE_LOG(LogTemp, Log, TEXT("Second icon image unhovered"));
    if (IconBorderSecond)
    {
        bIsSlotHover = false;
        AnimateBorderToNormal(IconBorderSecond, IconBorderSecondTimerHandle, 0.3f);
        OnSlotHover.Broadcast(bIsSlotHover);
    }
}

void UIconSlotWidget::OnIconImageThirdHovered()
{
    UE_LOG(LogTemp, Log, TEXT("Third icon image hovered"));
    if (IconBorderThird)
    {
        bIsSlotHover = true;
        AnimateBorderToHover(IconBorderThird, IconBorderThirdTimerHandle, 0.2f);
        OnSlotHover.Broadcast(bIsSlotHover);
    }
}

void UIconSlotWidget::OnIconImageThirdUnhovered()
{
    UE_LOG(LogTemp, Log, TEXT("Third icon image unhovered"));
    if (IconBorderThird)
    {
        bIsSlotHover = false;
        AnimateBorderToNormal(IconBorderThird, IconBorderThirdTimerHandle, 0.3f);
        OnSlotHover.Broadcast(bIsSlotHover);
    }
}

void UIconSlotWidget::AnimateBorderToColor(UBorder* Border, FTimerHandle& TimerHandle, const FLinearColor& TargetColor, TFunction<void()> OnFinished, float Duration)
{
    if (!IsValid(Border))
    {
        if (OnFinished) OnFinished();
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        if (OnFinished) OnFinished();
        return;
    }

    if (Duration <= 0.0f)
    {
        Border->SetBrushColor(TargetColor);
        if (OnFinished) OnFinished();
        return;
    }

    World->GetTimerManager().ClearTimer(TimerHandle);

    const FLinearColor StartColor = Border->GetBrushColor();
    TSharedPtr<float> Progress = MakeShareable(new float(0.0f));

    TWeakObjectPtr<UBorder> WeakBorder(Border);
    TWeakObjectPtr<UIconSlotWidget> WeakThis(this);

    FTimerDelegate AnimationDelegate;
    AnimationDelegate.BindLambda([WeakBorder, WeakThis, StartColor, TargetColor, Progress, Duration, OnFinished, &TimerHandle]() mutable
        {
            if (!WeakThis.IsValid() || !WeakBorder.IsValid())
            {
                if (WeakThis.IsValid())
                {
                    UWorld* LocalWorld = WeakThis->GetWorld();
                    if (LocalWorld) LocalWorld->GetTimerManager().ClearTimer(TimerHandle);
                }
                return;
            }

            UWorld* LocalWorld = WeakThis->GetWorld();
            if (!LocalWorld) return;

            *Progress += LocalWorld->GetDeltaSeconds() / Duration;
            *Progress = FMath::Clamp(*Progress, 0.0f, 1.0f);

            float SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, *Progress);
            FLinearColor CurrentColor = FMath::Lerp(StartColor, TargetColor, SmoothProgress);

            WeakBorder->SetBrushColor(CurrentColor);

            if (*Progress >= 1.0f)
            {
                LocalWorld->GetTimerManager().ClearTimer(TimerHandle);
                if (OnFinished) OnFinished();
            }
        });

    World->GetTimerManager().SetTimer(TimerHandle, AnimationDelegate, 0.016f, true);
}

void UIconSlotWidget::AnimateBorderToHover(UBorder* Border, FTimerHandle& TimerHandle, float Duration)
{
    if (Duration <= 0.0f) Duration = 0.2f;
    AnimateBorderToColor(Border, TimerHandle, HoverBorderColor, nullptr, Duration);
}

void UIconSlotWidget::AnimateBorderToNormal(UBorder* Border, FTimerHandle& TimerHandle, float Duration)
{
    if (Duration <= 0.0f) Duration = 0.3f;
    AnimateBorderToColor(Border, TimerHandle, NormalBorderColor, nullptr, Duration);
}