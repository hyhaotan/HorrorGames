// Fill out your copyright notice in the Description page of Project Settings.

#include "IconSlotContentWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/World.h"

void UIconSlotContentWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize content data for different icon indices
    IconTitles.Empty();
    IconTitles.Add(FText::FromString("Survival Instinct"));
    IconTitles.Add(FText::FromString("Fear Resistance"));
    IconTitles.Add(FText::FromString("Master of Horror"));

    IconDescriptions.Empty();
    IconDescriptions.Add(FText::FromString("You've proven your ability to survive in the darkest corners of this world. This achievement represents your first steps into true horror and your determination to keep going despite overwhelming fear."));
    IconDescriptions.Add(FText::FromString("Your mind has adapted to the constant terror. You've learned to control your fear and use it as a tool for survival. This level of mental fortitude is rare among players."));
    IconDescriptions.Add(FText::FromString("You have transcended fear itself. Few players reach this pinnacle of horror mastery. You've not only survived but thrived in the face of unimaginable terror."));

    IconStatsArray.Empty();
    IconStatsArray.Add(FText::FromString("Level: Beginner\nRarity: Common\nUnlocked: Yes\nProgress: 100%\nSurvival Time: 15 min"));
    IconStatsArray.Add(FText::FromString("Level: Intermediate\nRarity: Uncommon\nUnlocked: Yes\nProgress: 85%\nSurvival Time: 45 min"));
    IconStatsArray.Add(FText::FromString("Level: Master\nRarity: Legendary\nUnlocked: No\nProgress: 23%\nRequired: 120 min"));

    // Setup UI appearance
    SetupUI();

    // Play show animation
    PlayShowAnimation();

    UE_LOG(LogTemp, Log, TEXT("IconSlotContentWidget: Constructed"));
}

void UIconSlotContentWidget::NativeDestruct()
{
    // Clear animation timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AnimationTimer);
    }

    Super::NativeDestruct();

    UE_LOG(LogTemp, Log, TEXT("IconSlotContentWidget: Destructed"));
}

FReply UIconSlotContentWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // Check if click is outside the content area
    if (ContentBorder)
    {
        FGeometry BorderGeometry = ContentBorder->GetCachedGeometry();
        FVector2D LocalPosition = BorderGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        FVector2D BorderSize = BorderGeometry.GetLocalSize();

        // If click is outside the border, close the widget
        if (LocalPosition.X < 0 || LocalPosition.Y < 0 ||
            LocalPosition.X > BorderSize.X || LocalPosition.Y > BorderSize.Y)
        {
            return FReply::Handled();
        }
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UIconSlotContentWidget::SetIconContent(int32 ImageIndex)
{
    UpdateContentForImageIndex(ImageIndex);
}

void UIconSlotContentWidget::UpdateContentForImageIndex(int32 ImageIndex)
{
    // Validate index
    if (ImageIndex < 0 || ImageIndex >= IconTitles.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("IconSlotContentWidget: Invalid image index %d"), ImageIndex);
        return;
    }

    // Update title
    if (TitleText && IconTitles.IsValidIndex(ImageIndex))
    {
        TitleText->SetText(IconTitles[ImageIndex]);
    }

    // Update description
    if (DescriptionText && IconDescriptions.IsValidIndex(ImageIndex))
    {
        DescriptionText->SetText(IconDescriptions[ImageIndex]);
    }

    // Update stats
    if (StatsText && IconStatsArray.IsValidIndex(ImageIndex))
    {
        StatsText->SetText(IconStatsArray[ImageIndex]);
    }

    UE_LOG(LogTemp, Log, TEXT("IconSlotContentWidget: Updated content for image index %d"), ImageIndex);
}

void UIconSlotContentWidget::SetupUI()
{
    // Setup background
    if (ContentBorder)
    {
        ContentBorder->SetBrushColor(BackgroundColor);
    }

    // Setup text colors
    if (TitleText)
    {
        TitleText->SetColorAndOpacity(FSlateColor(TitleColor));
        TitleText->SetText(IconTitle);
    }

    if (DescriptionText)
    {
        DescriptionText->SetColorAndOpacity(FSlateColor(DescriptionColor));
        DescriptionText->SetText(IconDescription);
    }

    if (StatsText)
    {
        StatsText->SetColorAndOpacity(FSlateColor(StatsColor));
        StatsText->SetText(IconStats);
    }

    // Setup preview image if available
    if (IconPreviewImage)
    {
        IconPreviewImage->SetVisibility(ESlateVisibility::Visible);
    }
}

void UIconSlotContentWidget::PlayShowAnimation()
{
    if (!ContentBorder)
        return;

    // Start with small scale and transparent
    ContentBorder->SetRenderScale(FVector2D(0.1f, 0.1f));
    ContentBorder->SetRenderOpacity(0.0f);

    if (UWorld* World = GetWorld())
    {
        TSharedPtr<float> AnimProgress = MakeShareable(new float(0.0f));
        TWeakObjectPtr<UIconSlotContentWidget> WeakThis(this);

        FTimerDelegate AnimDelegate;
        AnimDelegate.BindLambda([WeakThis, AnimProgress]() {
            if (!WeakThis.IsValid())
                return;

            UWorld* LocalWorld = WeakThis->GetWorld();
            if (!LocalWorld)
                return;

            *AnimProgress += LocalWorld->GetDeltaSeconds() * 4.0f; // Animation speed
            *AnimProgress = FMath::Clamp(*AnimProgress, 0.0f, 1.0f);

            // Smooth scaling and fading
            float Scale = FMath::Lerp(0.1f, 1.0f, FMath::SmoothStep(0.0f, 1.0f, *AnimProgress));
            float Opacity = FMath::Lerp(0.0f, 1.0f, *AnimProgress);

            if (WeakThis->ContentBorder)
            {
                WeakThis->ContentBorder->SetRenderScale(FVector2D(Scale, Scale));
                WeakThis->ContentBorder->SetRenderOpacity(Opacity);
            }

            // Clear timer when animation is complete
            if (*AnimProgress >= 1.0f)
            {
                LocalWorld->GetTimerManager().ClearTimer(WeakThis->AnimationTimer);
            }
            });

        World->GetTimerManager().SetTimer(AnimationTimer, AnimDelegate, 0.016f, true);
    }
}

void UIconSlotContentWidget::PlayHideAnimation()
{
    if (!ContentBorder)
        return;

    if (UWorld* World = GetWorld())
    {
        TSharedPtr<float> AnimProgress = MakeShareable(new float(0.0f));
        TWeakObjectPtr<UIconSlotContentWidget> WeakThis(this);

        FTimerDelegate AnimDelegate;
        AnimDelegate.BindLambda([WeakThis, AnimProgress]() {
            if (!WeakThis.IsValid())
                return;

            UWorld* LocalWorld = WeakThis->GetWorld();
            if (!LocalWorld)
                return;

            *AnimProgress += LocalWorld->GetDeltaSeconds() * 6.0f; // Faster hide animation
            *AnimProgress = FMath::Clamp(*AnimProgress, 0.0f, 1.0f);

            // Scale down and fade out
            float Scale = FMath::Lerp(1.0f, 0.1f, *AnimProgress);
            float Opacity = FMath::Lerp(1.0f, 0.0f, *AnimProgress);

            if (WeakThis->ContentBorder)
            {
                WeakThis->ContentBorder->SetRenderScale(FVector2D(Scale, Scale));
                WeakThis->ContentBorder->SetRenderOpacity(Opacity);
            }

            // Clear timer when animation is complete
            if (*AnimProgress >= 1.0f)
            {
                LocalWorld->GetTimerManager().ClearTimer(WeakThis->AnimationTimer);
            }
            });

        World->GetTimerManager().SetTimer(AnimationTimer, AnimDelegate, 0.016f, true);
    }
}