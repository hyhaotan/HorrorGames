#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/TimerHandle.h"
#include "HorrorGame/Data/SettingsStructs.h"
#include "SettingsStateManager.generated.h"

class UHorrorGameSettings;
class UGameUserSettings;
class UMenuSettingSystemWidget;

UCLASS(BlueprintType, Blueprintable)
class HORRORGAME_API USettingsStateManager : public UObject
{
    GENERATED_BODY()

public:
    USettingsStateManager();

    // === INITIALIZATION ===

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void Initialize(UMenuSettingSystemWidget* InParentWidget);

    // === SETTINGS OPERATIONS ===

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void ApplySettings();

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void ResetCurrentTab(int32 TabIndex);

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void RestoreFactoryDefaults();

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void CancelChanges();

    // === STATE MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void MarkSettingsChanged();

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void CacheOriginalSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void RevertToOriginalSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void CacheTabSettings(int32 TabIndex);

    // === VALIDATION ===

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    FSettingsValidationResult ValidateSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    FSettingsValidationResult ValidateTabSettings(int32 TabIndex);

    // === GETTERS/SETTERS ===

    UFUNCTION(BlueprintPure, Category = "Settings State")
    bool HasPendingChanges() const { return bHasPendingChanges; }

    UFUNCTION(BlueprintCallable, Category = "Settings State")
    void SetPendingChanges(bool bPending);

    UFUNCTION(BlueprintPure, Category = "Settings State")
    bool IsLoading() const { return bIsLoading; }

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "Settings Events")
    FOnSettingsAppliedDelegate OnSettingsApplied;

    UPROPERTY(BlueprintAssignable, Category = "Settings Events")
    FOnSettingsResetDelegate OnSettingsReset;

    FOnSettingsValidationDelegate OnSettingsValidated;

protected:
    // === INTERNAL OPERATIONS ===

    void ApplySettingsInternal();
    void ResetTabSettingsInternal(int32 TabIndex);
    void RestoreFactoryDefaultsInternal();

    // === CONFIRMATION CALLBACKS ===

    UFUNCTION()
    void OnResetConfirmed(bool bConfirmed);

    UFUNCTION()
    void OnDefaultsConfirmed(bool bConfirmed);

    UFUNCTION()
    void OnCancelConfirmed(bool bConfirmed);

private:
    // === STATE VARIABLES ===

    UPROPERTY()
    bool bHasPendingChanges;

    UPROPERTY()
    bool bIsLoading;

    UPROPERTY()
    FString OriginalSettingsHash;

    // === PENDING OPERATIONS ===

    UPROPERTY()
    int32 PendingResetTabIndex;

    UPROPERTY()
    bool bPendingFactoryReset;

    // === REFERENCES ===

    UPROPERTY()
    TWeakObjectPtr<UMenuSettingSystemWidget> ParentWidget;

    UPROPERTY()
    TWeakObjectPtr<UHorrorGameSettings> GameSettings;

    UPROPERTY()
    TWeakObjectPtr<UGameUserSettings> UserSettings;

    // === TIMER HANDLES ===

    FTimerHandle ApplyTimerHandle;
    FTimerHandle ResetTimerHandle;
    FTimerHandle DefaultsTimerHandle;

    // === HELPER FUNCTIONS ===

    bool ValidateGameplaySettings(FString& OutErrorMessage);
    bool ValidateGraphicsSettings(FString& OutErrorMessage);
    bool ValidateAudioSettings(FString& OutErrorMessage);
    bool ValidateVoiceSettings(FString& OutErrorMessage);
    bool ValidateControlsSettings(FString& OutErrorMessage);
    bool ValidateAccessibilitySettings(FString& OutErrorMessage);

    void SetLoadingState(bool bLoading, const FString& Message = TEXT(""));
    void ShowNotification(const FString& Message, ENotificationType Type);
    void ShowConfirmationDialog(const FString& Title, const FString& Message, TFunction<void(bool)> Callback);

public:
    // === DEBUG FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Debug",CallInEditor)
    void DebugPrintCurrentState() const;

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugForceValidation();
};