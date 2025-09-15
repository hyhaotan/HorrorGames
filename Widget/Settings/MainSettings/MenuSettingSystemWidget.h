#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Sound/SoundCue.h"
#include "Animation/WidgetAnimation.h"
#include "HorrorGame/Data/SettingsStructs.h"
#include "MenuSettingSystemWidget.generated.h"

// Forward declarations for managers
class USettingsTabManager;
class USettingsStateManager;
class USettingsAudioManager;
class USettingsWidgetFactory;
class USettingsUIManager;
class USettingsInputHandler;

// Forward declarations for child widgets
class UGameplayWidget;
class UGraphicsWidget;
class UAudioWidget;
class UVoiceWidget;
class UControlsWidget;
class UAccessibilityWidget;
class UNotificationWidget;

UCLASS()
class HORRORGAME_API UMenuSettingSystemWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UMenuSettingSystemWidget(const FObjectInitializer& ObjectInitializer);

    // === PUBLIC INTERFACE (delegates to managers) ===

    UFUNCTION(BlueprintCallable, Category = "Settings Menu")
    void SwitchToTab(int32 TabIndex);

    UFUNCTION(BlueprintCallable, Category = "Settings Menu")
    void MarkSettingsChanged();

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    bool HasPendingChanges() const;

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    int32 GetCurrentTab() const;

    UFUNCTION(BlueprintCallable, Category = "Settings Menu")
    void ShowCustomNotification(const FString& Message, ENotificationType Type);

    // === MANAGER ACCESS ===

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    USettingsTabManager* GetTabManager() const { return TabManager; }

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    USettingsStateManager* GetStateManager() const { return StateManager; }

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    USettingsAudioManager* GetAudioManager() const { return AudioManager; }

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    USettingsWidgetFactory* GetWidgetFactory() const { return WidgetFactory; }

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    USettingsUIManager* GetUIManager() const { return UIManager; }

    // === CHILD WIDGET ACCESS (delegates to factory) ===

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    UGameplayWidget* GetGameplayWidget() const;

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    UGraphicsWidget* GetGraphicsWidget() const;

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    UAudioWidget* GetAudioWidget() const;

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    UVoiceWidget* GetVoiceWidget() const;

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    UControlsWidget* GetControlsWidget() const;

    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    UAccessibilityWidget* GetAccessibilityWidget() const;

    // === UI ELEMENT ACCESS (for managers) ===

    UWidgetSwitcher* GetContentSwitcher() const { return ContentSwitcher; }
    UTextBlock* GetTabDescriptionText() const { return TabDescriptionText; }
    UProgressBar* GetLoadingProgressBar() const { return LoadingProgressBar; }
    UTextBlock* GetLoadingMessageText() const { return LoadingMessageText; }
    UNotificationWidget* GetNotificationWidget() const { return NotificationWidget; }

    UButton* GetGameplayTabButton() const { return GameplayTabButton; }
    UButton* GetGraphicsTabButton() const { return GraphicsTabButton; }
    UButton* GetAudioTabButton() const { return AudioTabButton; }
    UButton* GetVoiceTabButton() const { return VoiceTabButton; }
    UButton* GetControlsTabButton() const { return ControlsTabButton; }
    UButton* GetAccessibilityTabButton() const { return AccessibilityTabButton; }

    UButton* GetApplyButton() const { return ApplyButton; }
    UButton* GetResetButton() const { return ResetButton; }
    UButton* GetBackButton() const { return BackButton; }
    UButton* GetDefaultsButton() const { return DefaultsButton; }
    UButton* GetCancelButton() const { return CancelButton; }

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "Settings Events")
    FOnSettingsAppliedDelegate OnSettingsAppliedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Settings Events")
    FOnSettingsResetDelegate OnSettingsResetDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Settings Events")
    FOnMenuClosedDelegate OnMenuClosedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Settings Events")
    FOnTabChangedDelegate OnTabChangedDelegate;

protected:
    // === UNREAL OVERRIDES ===

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // === UI ELEMENTS (BindWidget) ===

    UPROPERTY(meta = (BindWidget))
    UWidgetSwitcher* ContentSwitcher;

    // Tab Buttons
    UPROPERTY(meta = (BindWidget))
    UButton* GameplayTabButton;

    UPROPERTY(meta = (BindWidget))
    UButton* GraphicsTabButton;

    UPROPERTY(meta = (BindWidget))
    UButton* AudioTabButton;

    UPROPERTY(meta = (BindWidget))
    UButton* VoiceTabButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ControlsTabButton;

    UPROPERTY(meta = (BindWidget))
    UButton* AccessibilityTabButton;

    // Action Buttons
    UPROPERTY(meta = (BindWidget))
    UButton* ApplyButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ResetButton;

    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    UPROPERTY(meta = (BindWidget))
    UButton* DefaultsButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CancelButton;

    // UI Info Elements
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TabDescriptionText;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* LoadingProgressBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* LoadingMessageText;

    UPROPERTY(meta = (BindWidget))
    UNotificationWidget* NotificationWidget;

    // Animations
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* TabTransitionAnimation;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* FadeInAnimation;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* FadeOutAnimation;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ButtonHoverAnimation;

    // Sound Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* TabSwitchSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* HoverSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* ApplySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* ResetSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* BackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* ErrorSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* SettingsChangedSound;

    // === SIMPLE EVENT HANDLERS (delegate to managers) ===

    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnResetClicked();

    UFUNCTION()
    void OnBackClicked();

    UFUNCTION()
    void OnDefaultsClicked();

    UFUNCTION()
    void OnCancelClicked();

private:
    // === MANAGER OBJECTS ===

    UPROPERTY()
    USettingsTabManager* TabManager;

    UPROPERTY()
    USettingsStateManager* StateManager;

    UPROPERTY()
    USettingsAudioManager* AudioManager;

    UPROPERTY()
    USettingsWidgetFactory* WidgetFactory;

    UPROPERTY()
    USettingsUIManager* UIManager;

    UPROPERTY()
    USettingsInputHandler* InputHandler;

    // === INITIALIZATION HELPERS ===

    void CreateManagers();
    void InitializeManagers();
    void BindManagerEvents();
    void BindButtonEvents();
    void SetupAudioManager();

    // === MANAGER EVENT HANDLERS ===

    UFUNCTION()
    void OnManagerTabChanged(int32 NewTab, int32 OldTab);

    UFUNCTION()
    void OnManagerSettingsApplied();

    UFUNCTION()
    void OnManagerSettingsReset();

    UFUNCTION()
    void OnManagerFadeOutCompleted();

public:
    // === DEBUG FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Debug",CallInEditor)
    void DebugPrintManagerStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Debug",CallInEditor)
    void DebugWidgetHierarchy();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ForceRefreshAllTabs();
};