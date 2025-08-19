#include "PlayerProfileHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HorrorGame/Object/PlayerIDManager.h"
#include "Types/SlateVector2.h"
#include "Misc/AssertionMacros.h"
#include "IconSlotWidget.h"
#include "IconSlotContentWidget.h"

void UPlayerProfileHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get PlayerIDManager instance
    if (UWorld* World = GetWorld())
    {
        PlayerIDManager = NewObject<UPlayerIDManager>();
    }

    if (IconSlotWidget)
    {
        IconSlotWidget->OnIconClicked.AddDynamic(this, &UPlayerProfileHUDWidget::HandleIconClicked);
    }

    // Setup button bindings
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UPlayerProfileHUDWidget::OnCloseButtonClicked);
    }

    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UPlayerProfileHUDWidget::OnRefreshButtonClicked);
    }

    // Setup HUD appearance
    SetupHUDAppearance();

    // Update player information
    UpdatePlayerInfo();

    // Play show animation
    PlayShowAnimation();
}

void UPlayerProfileHUDWidget::NativeDestruct()
{
    // Clean up button bindings
    if (CloseButton)
    {
        CloseButton->OnClicked.RemoveAll(this);
    }

    if (RefreshButton)
    {
        RefreshButton->OnClicked.RemoveAll(this);
    }

    if (IconSlotWidget)
    {
        IconSlotWidget->OnIconClicked.RemoveDynamic(this, &UPlayerProfileHUDWidget::HandleIconClicked);
    }

    // Clear animation timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AnimTimer);
    }

    Super::NativeDestruct();
}

void UPlayerProfileHUDWidget::OnCloseButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("PlayerProfileHUDWidget: Close button clicked"));
    PlayHideAnimation();
    RemoveFromParent();
}

void UPlayerProfileHUDWidget::OnRefreshButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("PlayerProfileHUDWidget: Refresh button clicked"));
    if (PlayerIDManager)
    {
        PlayerIDManager->UpdateSteamName();
        UpdatePlayerInfo();
    }
}

FReply UPlayerProfileHUDWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FVector2D LocalMousePosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
    FVector2D WidgetSize = InGeometry.GetLocalSize();

    if (LocalMousePosition.X < 0 || LocalMousePosition.Y < 0 ||
        LocalMousePosition.X > WidgetSize.X || LocalMousePosition.Y > WidgetSize.Y)
    {
        OnCloseButtonClicked();
        return FReply::Handled();
    }

    return FReply::Handled();
}

void UPlayerProfileHUDWidget::UpdatePlayerInfo()
{
    if (!PlayerIDManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerProfileHUDWidget: PlayerIDManager is null"));
        return;
    }

    FPlayerProfile MyProfile = PlayerIDManager->GetMyProfile();

    if (MyProfile.PlayerID.IsEmpty())
    {
        if (PlayerIDManager->RegisterCurrentPlayer())
        {
            MyProfile = PlayerIDManager->GetMyProfile();
        }
    }

    if (PlayerNameText)
    {
        FString DisplayName = MyProfile.DisplayName.IsEmpty() ?
            PlayerIDManager->GetCurrentSteamName() : MyProfile.DisplayName;
        PlayerNameText->SetText(FText::FromString(DisplayName));
    }

    if (PlayerIDText)
    {
        FString PlayerID = MyProfile.PlayerID.IsEmpty() ?
            TEXT("Not Generated") : MyProfile.PlayerID;
        PlayerIDText->SetText(FText::FromString(FString::Printf(TEXT("Player ID: %s"), *PlayerID)));
    }

    if (SteamIDText)
    {
        FString SteamID = MyProfile.SteamID.IsEmpty() ?
            TEXT("Not Available") : MyProfile.SteamID;

        if (SteamID.Len() > 8)
        {
            SteamID = TEXT("...") + SteamID.Right(8);
        }

        SteamIDText->SetText(FText::FromString(FString::Printf(TEXT("Steam ID: %s"), *SteamID)));
    }

    if (CreatedDateText)
    {
        FString DateString = MyProfile.CreatedDate.ToString(TEXT("%Y-%m-%d %H:%M"));
        CreatedDateText->SetText(FText::FromString(FString::Printf(TEXT("Created: %s"), *DateString)));
    }

    UE_LOG(LogTemp, Log, TEXT("PlayerProfileHUDWidget: Player info updated - %s (%s)"),
        *MyProfile.DisplayName, *MyProfile.PlayerID);
}

void UPlayerProfileHUDWidget::SetupHUDAppearance()
{
    if (HUDBorder)
    {
        FLinearColor BackgroundColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.95f);
        HUDBorder->SetBrushColor(BackgroundColor);
    }

    if (PlayerNameText)
    {
        PlayerNameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 0.7f, 1.0f, 1.0f)));
    }

    if (PlayerIDText)
    {
        PlayerIDText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    }

    if (SteamIDText)
    {
        SteamIDText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));
    }

    if (CreatedDateText)
    {
        CreatedDateText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));
    }
}

void UPlayerProfileHUDWidget::PlayShowAnimation()
{
    if (HUDBorder)
    {
        HUDBorder->SetRenderScale(FVector2D(0.1f, 0.1f));
        HUDBorder->SetRenderOpacity(0.0f);

        if (UWorld* World = GetWorld())
        {
            TSharedPtr<float> AnimProgress = MakeShareable(new float(0.0f));
            TWeakObjectPtr<UPlayerProfileHUDWidget> WeakThis(this);

            FTimerDelegate AnimDelegate;
            AnimDelegate.BindLambda([WeakThis, AnimProgress]() {
                if (!WeakThis.IsValid()) return;

                UWorld* LocalWorld = WeakThis->GetWorld();
                if (!LocalWorld) return;

                *AnimProgress += LocalWorld->GetDeltaSeconds() * 5.0f;
                *AnimProgress = FMath::Clamp(*AnimProgress, 0.0f, 1.0f);

                float Scale = FMath::Lerp(0.1f, 1.0f, FMath::SmoothStep(0.0f, 1.0f, *AnimProgress));
                float Opacity = FMath::Lerp(0.0f, 1.0f, *AnimProgress);

                if (WeakThis->HUDBorder)
                {
                    WeakThis->HUDBorder->SetRenderScale(FVector2D(Scale, Scale));
                    WeakThis->HUDBorder->SetRenderOpacity(Opacity);
                }

                if (*AnimProgress >= 1.0f)
                {
                    LocalWorld->GetTimerManager().ClearTimer(WeakThis->AnimTimer);
                }
                });

            World->GetTimerManager().SetTimer(AnimTimer, AnimDelegate, 0.016f, true);
        }
    }
}

void UPlayerProfileHUDWidget::PlayHideAnimation()
{
    if (HUDBorder)
    {
        if (UWorld* World = GetWorld())
        {
            TSharedPtr<float> AnimProgress = MakeShareable(new float(0.0f));
            TWeakObjectPtr<UPlayerProfileHUDWidget> WeakThis(this);

            FTimerDelegate AnimDelegate;
            AnimDelegate.BindLambda([WeakThis, AnimProgress]() {
                if (!WeakThis.IsValid()) return;

                UWorld* LocalWorld = WeakThis->GetWorld();
                if (!LocalWorld) return;

                *AnimProgress += LocalWorld->GetDeltaSeconds() * 8.0f;
                *AnimProgress = FMath::Clamp(*AnimProgress, 0.0f, 1.0f);

                float Scale = FMath::Lerp(1.0f, 0.1f, *AnimProgress);
                float Opacity = FMath::Lerp(1.0f, 0.0f, *AnimProgress);

                if (WeakThis->HUDBorder)
                {
                    WeakThis->HUDBorder->SetRenderScale(FVector2D(Scale, Scale));
                    WeakThis->HUDBorder->SetRenderOpacity(Opacity);
                }

                if (*AnimProgress >= 1.0f)
                {
                    LocalWorld->GetTimerManager().ClearTimer(WeakThis->AnimTimer);
                }
                });

            World->GetTimerManager().SetTimer(AnimTimer, AnimDelegate, 0.016f, true);
        }
    }
}

void UPlayerProfileHUDWidget::HandleIconClicked(int32 ImageIndex, UIconSlotWidget* Source)
{
    UE_LOG(LogTemp, Log, TEXT("PlayerProfileHUDWidget: Icon clicked %d"), ImageIndex);
    ShowContentInProfileBox(ImageIndex);
}

void UPlayerProfileHUDWidget::ShowContentInProfileBox(int32 ImageIndex)
{
    if (!IconSlotContentWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerProfileHUDWidget: IconSlotContentWidgetClass not set"));
        return;
    }

    // Remove cũ nếu có
    HideProfileContent();

    // Tạo widget nội dung
    ProfileContentInstance = CreateWidget<UIconSlotContentWidget>(GetWorld(), IconSlotContentWidgetClass);
    if (!ProfileContentInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerProfileHUDWidget: Failed to create ProfileContentInstance"));
        return;
    }

    // Đặt vị trí / add vào ProfileInfoBox:
    if (ProfileInfoBoxBorder)
    {
        // Nếu ProfileInfoBoxBorder là Border có SetContent
        ProfileInfoBoxBorder->SetContent(ProfileContentInstance);
    }
    else
    {
        // Nếu bạn có Canvas/Panel khác, dùng AddChild
        ProfileContentInstance->AddToViewport(); // fallback: add to viewport (hoặc add vào panel nếu bạn có ref)
    }

    // Cập nhật nội dung tương ứng bằng hàm công khai có sẵn
    ProfileContentInstance->SetIconContent(ImageIndex);

    // (tùy chọn) play animation của content nếu widget có:
    // ProfileContentInstance->PlayShowAnimation(); // nếu method public
}

void UPlayerProfileHUDWidget::HideProfileContent()
{
    if (ProfileContentInstance)
    {
        ProfileContentInstance->RemoveFromParent();
        ProfileContentInstance = nullptr;
    }
}