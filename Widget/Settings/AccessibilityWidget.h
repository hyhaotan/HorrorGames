#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AccessibilityWidget.generated.h"

UCLASS()
class HORRORGAME_API UAccessibilityWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
    // Color Blind Support
    UPROPERTY(meta = (BindWidget))
    class UCheckBox* ColorBlindSupportCheckBox;

    UPROPERTY(meta = (BindWidget))
    class USelectionWidget* ColorBlindTypeSelection;

    // Font and UI
    UPROPERTY(meta = (BindWidget))
    class USlider* FontSizeSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* FontSizeText;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* HighContrastCheckBox;

    // Motion and Effects
    UPROPERTY(meta = (BindWidget))
    class UCheckBox* ReduceMotionCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* FlashingLightsCheckBox;

    UPROPERTY(meta = (BindWidget))
    class USlider* ScreenReaderSpeedSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ScreenReaderSpeedText;

private:
    void InitializeColorBlindSettings();
    void InitializeFontSettings();
    void InitializeMotionSettings();

    UFUNCTION()
    void OnColorBlindSupportChanged(bool bIsChecked);

    UFUNCTION()
    void OnFontSizeChanged(float Value);

    UFUNCTION()
    void OnHighContrastChanged(bool bIsChecked);

    UFUNCTION()
    void OnReduceMotionChanged(bool bIsChecked);

    UFUNCTION()
    void OnFlashingLightsChanged(bool bIsChecked);

    UFUNCTION()
    void OnScreenReaderSpeedChanged(float Value);

    class UHorrorGameSettings* HorrorGameSettings;
};