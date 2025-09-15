#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "GameplayWidget.generated.h"

// Forward declarations
class USelectionWidget;
class UGameUserSettings;
class UHorrorGameSettings;

UCLASS()
class HORRORGAME_API UGameplayWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UGameplayWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    UFUNCTION(BlueprintCallable, Category = "Gameplay Settings")
    void RefreshValues();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Settings")
    void ResetToDefaults();

protected:
    // Difficulty Settings
    UPROPERTY(meta = (BindWidget))
    class USelectionWidget* DifficultySelection;

    // Language Settings
    UPROPERTY(meta = (BindWidget))
    class USelectionWidget* LanguageSelection;

    // Gameplay Options - Checkboxes
    UPROPERTY(meta = (BindWidget))
    class UCheckBox* SubtitlesCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* AutoSaveCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* TutorialHintsCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* CameraShakeCheckBox;

    // UI Scale Settings
    UPROPERTY(meta = (BindWidget))
    class USlider* UIScaleSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* UIScaleValueText;

    // Interaction Hold Time Settings
    UPROPERTY(meta = (BindWidget))
    class USlider* InteractionHoldTimeSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* InteractionHoldTimeText;

private:
    // Initialization functions
    void InitializeDifficulty();
    void InitializeLanguage();
    void InitializeGameplayOptions();
    void BindAllEvents();

    // Event handlers for checkboxes
    UFUNCTION()
    void OnSubtitlesChanged(bool bIsChecked);

    UFUNCTION()
    void OnAutoSaveChanged(bool bIsChecked);

    UFUNCTION()
    void OnTutorialHintsChanged(bool bIsChecked);

    UFUNCTION()
    void OnCameraShakeChanged(bool bIsChecked);

    // Event handlers for sliders
    UFUNCTION()
    void OnUIScaleChanged(float Value);

    UFUNCTION()
    void OnInteractionHoldTimeChanged(float Value);

    // Settings references
    UPROPERTY()
    class UGameUserSettings* GameUserSettings;

    UPROPERTY()
    class UHorrorGameSettings* HorrorGameSettings;
};