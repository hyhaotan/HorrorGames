#include "SettingsUIManager.h"
#include "HorrorGame/Widget/Settings/MainSettings/MenuSettingSystemWidget.h"
#include "HorrorGame/Widget/Settings/Notification/NotificationWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Engine/World.h"
#include "TimerManager.h"

USettingsUIManager::USettingsUIManager()
    : ParentWidget(nullptr)
    , LoadingProgressBar(nullptr)
    , LoadingMessageText(nullptr)
    , NotificationArea(nullptr)
    , NotificationText(nullptr)
    , NotificationWidget(nullptr)
    , ApplyButton(nullptr)
    , ResetButton(nullptr)
    , BackButton(nullptr)
    , DefaultsButton(nullptr)
    , CancelButton(nullptr)
    , bIsLoading(false)
    , LoadingProgress(0.0f)
    , EnabledButtonColor(0.2f, 0.8f, 0.2f, 1.0f)
    , DisabledButtonColor(0.3f, 0.3f, 0.3f, 0.5f)
    , PendingChangesButtonColor(0.8f, 0.6f, 0.2f, 1.0f)
{
}

void USettingsUIManager::Initialize(UMenuSettingSystemWidget* InParentWidget)
{
    if (!InParentWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsUIManager::Initialize - ParentWidget is null!"));
        return;
    }

    ParentWidget = InParentWidget;

    // Cache UI element references
    LoadingProgressBar = InParentWidget->GetLoadingProgressBar();
    LoadingMessageText = InParentWidget->GetLoadingMessageText();
    NotificationWidget = InParentWidget->GetNotificationWidget();

    // Cache button references
    ApplyButton = InParentWidget->GetApplyButton();
    ResetButton = InParentWidget->GetResetButton();
    BackButton = InParentWidget->GetBackButton();
    DefaultsButton = InParentWidget->GetDefaultsButton();
    CancelButton = InParentWidget->GetCancelButton();

    // Initialize UI
    InitializeUI();

    UE_LOG(LogTemp, Log, TEXT("SettingsUIManager initialized"));
}

void USettingsUIManager::InitializeUI()
{
    // Initialize loading bar
    if (LoadingProgressBar.IsValid())
    {
        LoadingProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        LoadingProgressBar->SetPercent(0.0f);
    }

    // Initialize loading message
    if (LoadingMessageText.IsValid())
    {
        LoadingMessageText->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Initialize notification area
    if (NotificationArea.IsValid())
    {
        NotificationArea->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Set initial button states
    UpdateActionButtonStates(false, false);

    UE_LOG(LogTemp, Log, TEXT("SettingsUIManager: UI initialized"));
}

void USettingsUIManager::UpdateActionButtonStates(bool bHasPendingChanges, bool bIsLoadingState)
{
    // Apply button
    if (ApplyButton.IsValid())
    {
        ApplyButton->SetIsEnabled(bHasPendingChanges && !bIsLoadingState);
        FLinearColor ApplyColor = bHasPendingChanges ? EnabledButtonColor : DisabledButtonColor;
        ApplyButton->SetBackgroundColor(ApplyColor);
    }

    // Reset button
    if (ResetButton.IsValid())
    {
        ResetButton->SetIsEnabled(!bIsLoadingState);
        ResetButton->SetBackgroundColor(PendingChangesButtonColor);
    }

    // Defaults button
    if (DefaultsButton.IsValid())
    {
        DefaultsButton->SetIsEnabled(!bIsLoadingState);
        DefaultsButton->SetBackgroundColor(FLinearColor(0.8f, 0.4f, 0.2f, 1.0f));
    }

    // Cancel button
    if (CancelButton.IsValid())
    {
        CancelButton->SetIsEnabled(!bIsLoadingState);
        FLinearColor CancelColor = bHasPendingChanges ?
            FLinearColor(0.8f, 0.3f, 0.3f, 1.0f) :
            FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
        CancelButton->SetBackgroundColor(CancelColor);
    }

    // Back button
    if (BackButton.IsValid())
    {
        BackButton->SetIsEnabled(!bIsLoadingState);
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsUIManager: Button states updated - Pending: %s, Loading: %s"),
        bHasPendingChanges ? TEXT("true") : TEXT("false"),
        bIsLoadingState ? TEXT("true") : TEXT("false"));
}

void USettingsUIManager::SetButtonsEnabled(bool bEnabled)
{
    TArray<TWeakObjectPtr<UButton>> AllButtons = {
        ApplyButton, ResetButton, BackButton, DefaultsButton, CancelButton
    };

    for (auto ButtonPtr : AllButtons)
    {
        if (ButtonPtr.IsValid())
        {
            ButtonPtr->SetIsEnabled(bEnabled);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsUIManager: All buttons %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void USettingsUIManager::SetLoadingState(bool bLoading, const FString& Message)
{
    bIsLoading = bLoading;

    if (LoadingProgressBar.IsValid())
    {
        LoadingProgressBar->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

        if (bLoading)
        {
            LoadingProgress = 0.0f;
            LoadingProgressBar->SetPercent(0.0f);
            StartLoadingAnimation();
        }
        else
        {
            StopLoadingAnimation();
        }
    }

    if (LoadingMessageText.IsValid())
    {
        if (bLoading)
        {
            LoadingMessageText->SetText(FText::FromString(Message));
            LoadingMessageText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            LoadingMessageText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsUIManager: Loading state set to %s - %s"),
        bLoading ? TEXT("true") : TEXT("false"), *Message);
}

void USettingsUIManager::ShowNotification(const FString& Message, ENotificationType Type)
{
    if (NotificationWidget.IsValid())
    {
        NotificationWidget->ShowNotification(Message, Type);
    }
    else
    {
        ShowFallbackNotification(Message, Type);
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsUIManager: Notification shown - %s"), *Message);
}

void USettingsUIManager::ShowFallbackNotification(const FString& Message, ENotificationType Type)
{
    if (NotificationArea.IsValid() && NotificationText.IsValid())
    {
        NotificationText->SetText(FText::FromString(Message));
        NotificationText->SetColorAndOpacity(GetNotificationColor(Type));
        NotificationArea->SetVisibility(ESlateVisibility::Visible);

        AutoHideNotification();
    }
}

void USettingsUIManager::ShowConfirmationDialog(const FString& Title, const FString& Message, TFunction<void(bool)> Callback)
{
    UE_LOG(LogTemp, Warning, TEXT("SettingsUIManager: Confirmation Dialog - %s: %s"), *Title, *Message);
    UE_LOG(LogTemp, Warning, TEXT("TODO: Implement proper confirmation dialog widget"));

    // For now, auto-confirm after brief delay for testing
    if (UWorld* World = GetWorld())
    {
        FTimerHandle ConfirmTimer;
        World->GetTimerManager().SetTimer(ConfirmTimer, [Callback]()
            {
                if (Callback)
                {
                    Callback(true); // Auto-confirm for testing
                }
            }, 0.1f, false);
    }
    else if (Callback)
    {
        Callback(true);
    }
}

void USettingsUIManager::HideAllNotifications()
{
    if (NotificationArea.IsValid())
    {
        NotificationArea->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (NotificationWidget.IsValid())
    {
        // Assume NotificationWidget has a Hide method
        // NotificationWidget->Hide();
    }

    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(NotificationTimerHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsUIManager: All notifications hidden"));
}

// === HELPER FUNCTIONS ===

FLinearColor USettingsUIManager::GetNotificationColor(ENotificationType Type) const
{
    switch (Type)
    {
    case ENotificationType::Success:
        return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
    case ENotificationType::Warning:
        return FLinearColor(0.8f, 0.6f, 0.2f, 1.0f);
    case ENotificationType::Error:
        return FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);
    case ENotificationType::Info:
    default:
        return FLinearColor(0.2f, 0.6f, 0.8f, 1.0f);
    }
}

void USettingsUIManager::AutoHideNotification()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(NotificationTimerHandle, [this]()
            {
                if (NotificationArea.IsValid())
                {
                    NotificationArea->SetVisibility(ESlateVisibility::Collapsed);
                }
            }, 3.0f, false);
    }
}

void USettingsUIManager::StartLoadingAnimation()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(LoadingAnimationHandle, this, &USettingsUIManager::UpdateLoadingProgress, 0.05f, true);
    }
}

void USettingsUIManager::StopLoadingAnimation()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LoadingAnimationHandle);
    }
    LoadingProgress = 0.0f;
}

void USettingsUIManager::UpdateLoadingProgress()
{
    if (LoadingProgressBar.IsValid() && bIsLoading)
    {
        LoadingProgress += 0.02f; // Increment by 2% each tick
        LoadingProgress = FMath::Min(LoadingProgress, 1.0f);
        LoadingProgressBar->SetPercent(LoadingProgress);

        if (LoadingProgress >= 1.0f)
        {
            StopLoadingAnimation();
        }
    }
}

// === DEBUG FUNCTIONS ===

void USettingsUIManager::DebugPrintUIState() const
{
    UE_LOG(LogTemp, Log, TEXT("=== SettingsUIManager Debug Info ==="));
    UE_LOG(LogTemp, Log, TEXT("Is Loading: %s"), bIsLoading ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("Loading Progress: %f"), LoadingProgress);
    UE_LOG(LogTemp, Log, TEXT("LoadingProgressBar: %s"), LoadingProgressBar.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
    UE_LOG(LogTemp, Log, TEXT("LoadingMessageText: %s"), LoadingMessageText.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
    UE_LOG(LogTemp, Log, TEXT("NotificationArea: %s"), NotificationArea.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
    UE_LOG(LogTemp, Log, TEXT("NotificationWidget: %s"), NotificationWidget.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
    UE_LOG(LogTemp, Log, TEXT("ApplyButton: %s"), ApplyButton.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
    UE_LOG(LogTemp, Log, TEXT("ParentWidget: %s"), ParentWidget.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
    UE_LOG(LogTemp, Log, TEXT("=== End Debug Info ==="));
}

void USettingsUIManager::DebugTestNotifications()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsUIManager: Testing all notification types..."));

    ShowNotification(TEXT("This is an INFO notification"), ENotificationType::Info);

    // Add delays for testing
    if (UWorld* World = GetWorld())
    {
        FTimerHandle Timer1, Timer2, Timer3;

        World->GetTimerManager().SetTimer(Timer1, [this]()
            {
                ShowNotification(TEXT("This is a SUCCESS notification"), ENotificationType::Success);
            }, 1.0f, false);

        World->GetTimerManager().SetTimer(Timer2, [this]()
            {
                ShowNotification(TEXT("This is a WARNING notification"), ENotificationType::Warning);
            }, 2.0f, false);

        World->GetTimerManager().SetTimer(Timer3, [this]()
            {
                ShowNotification(TEXT("This is an ERROR notification"), ENotificationType::Error);
            }, 3.0f, false);
    }
}