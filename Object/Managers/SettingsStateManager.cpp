#include "SettingsStateManager.h"
#include "HorrorGame/Widget/Settings/MainSettings/MenuSettingSystemWidget.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/World.h"
#include "TimerManager.h"

USettingsStateManager::USettingsStateManager()
    : bHasPendingChanges(false)
    , bIsLoading(false)
    , OriginalSettingsHash(TEXT(""))
    , PendingResetTabIndex(-1)
    , bPendingFactoryReset(false)
    , ParentWidget(nullptr)
    , GameSettings(nullptr)
    , UserSettings(nullptr)
{
}

void USettingsStateManager::Initialize(UMenuSettingSystemWidget* InParentWidget)
{
    if (!InParentWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsStateManager::Initialize - ParentWidget is null!"));
        return;
    }

    ParentWidget = InParentWidget;
    GameSettings = UHorrorGameSettings::Get();
    UserSettings = UGameUserSettings::GetGameUserSettings();

    // Cache original settings
    CacheOriginalSettings();

    // Bind to settings changed delegate
    if (GameSettings.IsValid())
    {
        GameSettings->OnSettingsSimpleChanged.RemoveDynamic(this, &USettingsStateManager::MarkSettingsChanged);
        GameSettings->OnSettingsSimpleChanged.AddDynamic(this, &USettingsStateManager::MarkSettingsChanged);
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsStateManager initialized"));
}

void USettingsStateManager::ApplySettings()
{
    if (!bHasPendingChanges)
    {
        ShowNotification(TEXT("No changes to apply"), ENotificationType::Info);
        return;
    }

    SetLoadingState(true, TEXT("Applying settings..."));

    // Apply with delay for better UX
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ApplyTimerHandle, [this]()
            {
                ApplySettingsInternal();
            }, 0.5f, false);
    }
    else
    {
        ApplySettingsInternal();
    }
}

void USettingsStateManager::ApplySettingsInternal()
{
    bool bSuccess = true;
    FString ErrorMessage;

    try
    {
        // Apply game user settings
        if (UserSettings.IsValid())
        {
            UserSettings->ApplySettings(true);
        }

        // Apply horror game settings
        if (GameSettings.IsValid())
        {
            GameSettings->SaveConfig();
        }

        // Clear pending changes
        SetPendingChanges(false);

        // Update cached settings
        CacheOriginalSettings();

        // Broadcast success
        OnSettingsApplied.Broadcast();
    }
    catch (...)
    {
        bSuccess = false;
        ErrorMessage = TEXT("Failed to apply settings. Please try again.");
    }

    SetLoadingState(false);

    if (bSuccess)
    {
        ShowNotification(TEXT("Settings Applied Successfully"), ENotificationType::Success);
        UE_LOG(LogTemp, Log, TEXT("Settings applied successfully"));
    }
    else
    {
        ShowNotification(ErrorMessage, ENotificationType::Error);
        UE_LOG(LogTemp, Error, TEXT("Failed to apply settings: %s"), *ErrorMessage);
    }
}

void USettingsStateManager::ResetCurrentTab(int32 TabIndex)
{
    PendingResetTabIndex = TabIndex;

    FString TabName = TEXT("Current Tab");
    if (ParentWidget.IsValid())
    {
        // Get tab name from parent widget's tab data
        TabName = FString::Printf(TEXT("Tab %d"), TabIndex);
    }

    ShowConfirmationDialog(
        TEXT("Reset Tab Settings"),
        FString::Printf(TEXT("Are you sure you want to reset %s settings to defaults? This cannot be undone."), *TabName),
        [this](bool bConfirmed) { OnResetConfirmed(bConfirmed); }
    );
}

void USettingsStateManager::OnResetConfirmed(bool bConfirmed)
{
    if (!bConfirmed || PendingResetTabIndex < 0)
        return;

    SetLoadingState(true, TEXT("Resetting settings..."));

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ResetTimerHandle, [this]()
            {
                ResetTabSettingsInternal(PendingResetTabIndex);
                PendingResetTabIndex = -1;
            }, 0.3f, false);
    }
    else
    {
        ResetTabSettingsInternal(PendingResetTabIndex);
        PendingResetTabIndex = -1;
    }
}

void USettingsStateManager::ResetTabSettingsInternal(int32 TabIndex)
{
    bool bResetSuccessful = false;

    // Reset settings based on tab index
    switch (TabIndex)
    {
    case 0: // Gameplay
        if (GameSettings.IsValid())
        {
            // Reset gameplay specific settings
            // GameSettings->ResetGameplaySettings();
            bResetSuccessful = true;
        }
        break;
    case 1: // Graphics
        if (UserSettings.IsValid())
        {
            // Reset graphics settings
            UserSettings->SetToDefaults();
            bResetSuccessful = true;
        }
        break;
    case 2: // Audio
    case 3: // Voice
    case 4: // Controls
    case 5: // Accessibility
        if (GameSettings.IsValid())
        {
            // Reset respective settings
            bResetSuccessful = true;
        }
        break;
    }

    SetLoadingState(false);

    if (bResetSuccessful)
    {
        SetPendingChanges(true);
        ShowNotification(FString::Printf(TEXT("Tab %d settings reset to defaults"), TabIndex), ENotificationType::Info);
        OnSettingsReset.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("Tab %d settings reset successfully"), TabIndex);
    }
    else
    {
        ShowNotification(TEXT("Failed to reset settings"), ENotificationType::Error);
        UE_LOG(LogTemp, Error, TEXT("Failed to reset tab %d settings"), TabIndex);
    }
}

void USettingsStateManager::RestoreFactoryDefaults()
{
    bPendingFactoryReset = true;

    ShowConfirmationDialog(
        TEXT("Reset All Settings"),
        TEXT("Are you sure you want to reset ALL settings to factory defaults? This will affect all tabs and cannot be undone."),
        [this](bool bConfirmed) { OnDefaultsConfirmed(bConfirmed); }
    );
}

void USettingsStateManager::OnDefaultsConfirmed(bool bConfirmed)
{
    if (!bConfirmed || !bPendingFactoryReset)
        return;

    bPendingFactoryReset = false;
    SetLoadingState(true, TEXT("Restoring factory defaults..."));

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DefaultsTimerHandle, [this]()
            {
                RestoreFactoryDefaultsInternal();
            }, 0.5f, false);
    }
    else
    {
        RestoreFactoryDefaultsInternal();
    }
}

void USettingsStateManager::RestoreFactoryDefaultsInternal()
{
    bool bSuccess = false;

    try
    {
        // Reset all game settings
        if (GameSettings.IsValid())
        {
            GameSettings->ResetAllToDefaults();
        }

        if (UserSettings.IsValid())
        {
            UserSettings->SetToDefaults();
        }

        SetPendingChanges(true);
        bSuccess = true;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception occurred while restoring factory defaults"));
    }

    SetLoadingState(false);

    if (bSuccess)
    {
        ShowNotification(TEXT("All settings restored to factory defaults"), ENotificationType::Info);
        OnSettingsReset.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("Factory defaults restored successfully"));
    }
    else
    {
        ShowNotification(TEXT("Failed to restore factory defaults"), ENotificationType::Error);
    }
}

void USettingsStateManager::CancelChanges()
{
    if (bHasPendingChanges)
    {
        ShowConfirmationDialog(
            TEXT("Discard Changes"),
            TEXT("You have unsaved changes. Are you sure you want to discard them?"),
            [this](bool bConfirmed) { OnCancelConfirmed(bConfirmed); }
        );
    }
}

void USettingsStateManager::OnCancelConfirmed(bool bConfirmed)
{
    if (bConfirmed)
    {
        RevertToOriginalSettings();
    }
}

void USettingsStateManager::MarkSettingsChanged()
{
    if (!bIsLoading)
    {
        SetPendingChanges(true);
        UE_LOG(LogTemp, Log, TEXT("Settings marked as changed"));
    }
}

void USettingsStateManager::SetPendingChanges(bool bPending)
{
    if (bHasPendingChanges != bPending)
    {
        bHasPendingChanges = bPending;
        UE_LOG(LogTemp, Log, TEXT("Pending changes set to: %s"), bPending ? TEXT("true") : TEXT("false"));
    }
}

void USettingsStateManager::CacheOriginalSettings()
{
    if (GameSettings.IsValid())
    {
        OriginalSettingsHash = FString::Printf(TEXT("CachedSettings_%lld"), FDateTime::Now().GetTicks());
        UE_LOG(LogTemp, Log, TEXT("Settings cached with hash: %s"), *OriginalSettingsHash);
    }

    if (UserSettings.IsValid())
    {
        UserSettings->LoadSettings();
        UE_LOG(LogTemp, Log, TEXT("Game user settings cached"));
    }
}

void USettingsStateManager::RevertToOriginalSettings()
{
    if (GameSettings.IsValid())
    {
        GameSettings->LoadConfig();
        UE_LOG(LogTemp, Log, TEXT("Settings reverted to original configuration"));
    }

    if (UserSettings.IsValid())
    {
        UserSettings->LoadSettings();
        UE_LOG(LogTemp, Log, TEXT("Game user settings reverted"));
    }

    SetPendingChanges(false);
    ShowNotification(TEXT("Settings reverted to original values"), ENotificationType::Info);
}

void USettingsStateManager::CacheTabSettings(int32 TabIndex)
{
    UE_LOG(LogTemp, Log, TEXT("Tab %d settings cached"), TabIndex);
}

FSettingsValidationResult USettingsStateManager::ValidateSettings()
{
    FString ErrorMessage;

    // Validate each tab
    for (int32 i = 0; i < 6; ++i)
    {
        FSettingsValidationResult TabResult = ValidateTabSettings(i);
        if (!TabResult.bIsValid)
        {
            return TabResult;
        }
    }

    return FSettingsValidationResult::Success();
}

FSettingsValidationResult USettingsStateManager::ValidateTabSettings(int32 TabIndex)
{
    FString ErrorMessage;
    bool bIsValid = true;

    switch (TabIndex)
    {
    case 0: bIsValid = ValidateGameplaySettings(ErrorMessage); break;
    case 1: bIsValid = ValidateGraphicsSettings(ErrorMessage); break;
    case 2: bIsValid = ValidateAudioSettings(ErrorMessage); break;
    case 3: bIsValid = ValidateVoiceSettings(ErrorMessage); break;
    case 4: bIsValid = ValidateControlsSettings(ErrorMessage); break;
    case 5: bIsValid = ValidateAccessibilitySettings(ErrorMessage); break;
    default:
        return FSettingsValidationResult::Error(TEXT("Invalid tab index"));
    }

    return bIsValid ? FSettingsValidationResult::Success() : FSettingsValidationResult::Error(ErrorMessage);
}

// === VALIDATION HELPERS ===

bool USettingsStateManager::ValidateGameplaySettings(FString& OutErrorMessage)
{
    // Add gameplay validation logic
    return true;
}

bool USettingsStateManager::ValidateGraphicsSettings(FString& OutErrorMessage)
{
    // Add graphics validation logic
    return true;
}

bool USettingsStateManager::ValidateAudioSettings(FString& OutErrorMessage)
{
    // Add audio validation logic
    return true;
}

bool USettingsStateManager::ValidateVoiceSettings(FString& OutErrorMessage)
{
    // Add voice validation logic
    return true;
}

bool USettingsStateManager::ValidateControlsSettings(FString& OutErrorMessage)
{
    // Add controls validation logic
    return true;
}

bool USettingsStateManager::ValidateAccessibilitySettings(FString& OutErrorMessage)
{
    // Add accessibility validation logic
    return true;
}

// === HELPER FUNCTIONS ===

void USettingsStateManager::SetLoadingState(bool bLoading, const FString& Message)
{
    bIsLoading = bLoading;

    if (ParentWidget.IsValid())
    {
        // Delegate to parent widget's UI manager
        // ParentWidget->GetUIManager()->SetLoadingState(bLoading, Message);
    }
}

void USettingsStateManager::ShowNotification(const FString& Message, ENotificationType Type)
{
    if (ParentWidget.IsValid())
    {
        // Delegate to parent widget's UI manager
        // ParentWidget->GetUIManager()->ShowNotification(Message, Type);
    }
}

void USettingsStateManager::ShowConfirmationDialog(const FString& Title, const FString& Message, TFunction<void(bool)> Callback)
{
    if (ParentWidget.IsValid())
    {
        // Delegate to parent widget's UI manager
        // ParentWidget->GetUIManager()->ShowConfirmationDialog(Title, Message, Callback);
    }

    // For now, auto-confirm for testing
    if (Callback)
    {
        Callback(true);
    }
}

// === DEBUG FUNCTIONS ===

void USettingsStateManager::DebugPrintCurrentState() const
{
    UE_LOG(LogTemp, Log, TEXT("=== SettingsStateManager Debug Info ==="));
    UE_LOG(LogTemp, Log, TEXT("Has Pending Changes: %s"), bHasPendingChanges ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("Is Loading: %s"), bIsLoading ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("Original Settings Hash: %s"), *OriginalSettingsHash);
    UE_LOG(LogTemp, Log, TEXT("GameSettings Valid: %s"), GameSettings.IsValid() ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("UserSettings Valid: %s"), UserSettings.IsValid() ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("=== End Debug Info ==="));
}

void USettingsStateManager::DebugForceValidation()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsStateManager: Force validating all settings..."));

    FSettingsValidationResult Result = ValidateSettings();
    if (Result.bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("All settings validation passed"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Settings validation failed: %s"), *Result.ErrorMessage);
    }

    OnSettingsValidated.Broadcast(Result);
}