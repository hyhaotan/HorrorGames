#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Engine/TimerHandle.h"
#include "HorrorGame/Data/SettingsStructs.h"
#include "SettingsUIManager.generated.h"

class UMenuSettingSystemWidget;
class UNotificationWidget;

UCLASS(BlueprintType, Blueprintable)
class HORRORGAME_API USettingsUIManager : public UObject
{
    GENERATED_BODY()

public:
    USettingsUIManager();

    // === INITIALIZATION ===

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void Initialize(UMenuSettingSystemWidget* InParentWidget);

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void InitializeUI();

    // === BUTTON STATE MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void UpdateActionButtonStates(bool bHasPendingChanges, bool bIsLoading);

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void SetButtonsEnabled(bool bEnabled);

    // === LOADING STATE ===

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void SetLoadingState(bool bLoading, const FString& Message = TEXT(""));

    // === NOTIFICATIONS ===

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void ShowNotification(const FString& Message, ENotificationType Type);

    void ShowConfirmationDialog(const FString& Title, const FString& Message, TFunction<void(bool)> Callback);

    UFUNCTION(BlueprintCallable, Category = "UI Manager")
    void HideAllNotifications();

    // === UI ELEMENT GETTERS ===

    UFUNCTION(BlueprintPure, Category = "UI Manager")
    bool IsLoading() const { return bIsLoading; }

protected:
    // === NOTIFICATION HELPERS ===

    void ShowFallbackNotification(const FString& Message, ENotificationType Type);
    FLinearColor GetNotificationColor(ENotificationType Type) const;
    void AutoHideNotification();

    // === LOADING ANIMATION ===

    void StartLoadingAnimation();
    void StopLoadingAnimation();

    UFUNCTION()
    void UpdateLoadingProgress();

private:
    // === REFERENCES ===

    UPROPERTY()
    TWeakObjectPtr<UMenuSettingSystemWidget> ParentWidget;

    // === UI ELEMENT REFERENCES ===

    UPROPERTY()
    TWeakObjectPtr<UProgressBar> LoadingProgressBar;

    UPROPERTY()
    TWeakObjectPtr<UTextBlock> LoadingMessageText;

    UPROPERTY()
    TWeakObjectPtr<UBorder> NotificationArea;

    UPROPERTY()
    TWeakObjectPtr<UTextBlock> NotificationText;

    UPROPERTY()
    TWeakObjectPtr<UNotificationWidget> NotificationWidget;

    // === ACTION BUTTON REFERENCES ===

    UPROPERTY()
    TWeakObjectPtr<UButton> ApplyButton;

    UPROPERTY()
    TWeakObjectPtr<UButton> ResetButton;

    UPROPERTY()
    TWeakObjectPtr<UButton> BackButton;

    UPROPERTY()
    TWeakObjectPtr<UButton> DefaultsButton;

    UPROPERTY()
    TWeakObjectPtr<UButton> CancelButton;

    // === STATE VARIABLES ===

    UPROPERTY()
    bool bIsLoading;

    UPROPERTY()
    float LoadingProgress;

    // === TIMER HANDLES ===

    FTimerHandle NotificationTimerHandle;
    FTimerHandle LoadingAnimationHandle;

    // === STYLING PROPERTIES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling", meta = (AllowPrivateAccess = "true"))
    FLinearColor EnabledButtonColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling", meta = (AllowPrivateAccess = "true"))
    FLinearColor DisabledButtonColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling", meta = (AllowPrivateAccess = "true"))
    FLinearColor PendingChangesButtonColor;

public:
    // === DEBUG FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugPrintUIState() const;

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugTestNotifications();
};