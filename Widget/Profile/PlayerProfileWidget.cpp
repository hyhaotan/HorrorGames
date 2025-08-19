#include "PlayerProfileWidget.h"
#include "PlayerProfileHUDWidget.h"
#include "HorrorGame/Object/PlayerIDManager.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Animation/UMGSequencePlayer.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UPlayerProfileWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get PlayerIDManager instance
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            PlayerIDManager = NewObject<UPlayerIDManager>();
        }
    }

    // Initialize border color
    if (BorderImage)
    {
        BorderImage->SetBrushColor(NormalBorderColor);
    }

    // Update player name
    UpdatePlayerName();

    // Initialize HUD widget as null
    CurrentHUDWidget = nullptr;
}

FReply UPlayerProfileWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // Handle left mouse button click
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        ShowPlayerHUD();
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UPlayerProfileWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    // Animate border to hover state
    AnimateBorderToHover();

    // Update player name in case it changed
    UpdatePlayerName();
}

void UPlayerProfileWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    // Animate border back to normal state
    AnimateBorderToNormal();
}

void UPlayerProfileWidget::UpdatePlayerName()
{
    if (!PlayerIDManager || !DisplayNamePlayer)
    {
        return;
    }

    // Get current Steam name
    FString SteamName = PlayerIDManager->GetCurrentSteamName();

    if (!SteamName.IsEmpty() && SteamName != TEXT("Unknown Player"))
    {
        DisplayNamePlayer->SetText(FText::FromString(SteamName));
    }
    else
    {
        DisplayNamePlayer->SetText(FText::FromString(TEXT("Player")));
    }
}

void UPlayerProfileWidget::ShowPlayerHUD()
{
    // Hide existing HUD if present
    if (CurrentHUDWidget && CurrentHUDWidget->IsInViewport())
    {
        HidePlayerHUD();
        return;
    }

    // Create and show new HUD widget
    if (PlayerHUDClass)
    {
        CurrentHUDWidget = CreateWidget<UPlayerProfileHUDWidget>(GetWorld(), PlayerHUDClass);
        if (CurrentHUDWidget)
        {
            CurrentHUDWidget->AddToViewport(999);

            UE_LOG(LogTemp, Log, TEXT("PlayerProfileWidget: HUD widget shown"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerProfileWidget: PlayerHUDClass is not set!"));
    }
}

void UPlayerProfileWidget::HidePlayerHUD()
{
    if (CurrentHUDWidget)
    {
        CurrentHUDWidget->RemoveFromParent();
        CurrentHUDWidget = nullptr;

        UE_LOG(LogTemp, Log, TEXT("PlayerProfileWidget: HUD widget hidden"));
    }
}

void UPlayerProfileWidget::AnimateBorderToHover()
{
    if (!BorderImage)
    {
        return;
    }

    // Create color animation from current to hover color
    FLinearColor StartColor = BorderImage->GetBrushColor();

    if (UWorld* World = GetWorld())
    {
        float ElapsedTime = 0.0f;

        // Simple color lerp animation
        World->GetTimerManager().SetTimer(TimerHandle, [this, StartColor]()
            {
                static float Alpha = 0.0f;
                Alpha += GetWorld()->GetDeltaSeconds() / HoverAnimationDuration;
                Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

                FLinearColor CurrentColor = FMath::Lerp(StartColor, HoverBorderColor, Alpha);
                BorderImage->SetBrushColor(CurrentColor);

                if (Alpha >= 1.0f)
                {
                    Alpha = 0.0f;
                    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
                    OnBorderHoverAnimationFinished();
                }
            }, 0.016f, true);
    }
    else
    {
        // Fallback: immediate color change
        BorderImage->SetBrushColor(HoverBorderColor);
    }
}

void UPlayerProfileWidget::AnimateBorderToNormal()
{
    if (!BorderImage)
    {
        return;
    }

    // Create color animation from current to normal color
    FLinearColor StartColor = BorderImage->GetBrushColor();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TimerHandle, [this, StartColor]()
            {
                static float Alpha = 0.0f;
                Alpha += GetWorld()->GetDeltaSeconds() / HoverAnimationDuration;
                Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

                FLinearColor CurrentColor = FMath::Lerp(StartColor, NormalBorderColor, Alpha);
                BorderImage->SetBrushColor(CurrentColor);

                if (Alpha >= 1.0f)
                {
                    Alpha = 0.0f;
                    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
                    OnBorderNormalAnimationFinished();
                }
            }, 0.016f, true);
    }
    else
    {
        // Fallback: immediate color change
        BorderImage->SetBrushColor(NormalBorderColor);
    }
}

void UPlayerProfileWidget::OnBorderHoverAnimationFinished()
{
    // Optional: Add any post-hover animation logic here
    UE_LOG(LogTemp, Verbose, TEXT("PlayerProfileWidget: Hover animation finished"));
}

void UPlayerProfileWidget::OnBorderNormalAnimationFinished()
{
    // Optional: Add any post-normal animation logic here
    UE_LOG(LogTemp, Verbose, TEXT("PlayerProfileWidget: Normal animation finished"));
}