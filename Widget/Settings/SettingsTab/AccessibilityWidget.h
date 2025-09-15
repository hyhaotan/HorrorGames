#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AccessibilityWidget.generated.h"

class USelectionWidget;
class UCheckBox;
class USlider;
class UTextBlock;

UCLASS()
class HORRORGAME_API UAccessibilityWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    UFUNCTION(BlueprintCallable, Category = "Accessibility Settings")
    void RefreshValues();

    UFUNCTION(BlueprintCallable, Category = "Accessibility Settings")
    void ResetToDefaults();

protected:
    // Color Blind Support
    UPROPERTY(meta = (BindWidget))
    UCheckBox* ColorBlindSupportCheckBox;

    UPROPERTY(meta = (BindWidget))
    USelectionWidget* ColorBlindTypeSelection;

    // Font and UI
    UPROPERTY(meta = (BindWidget))
    USlider* FontSizeSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* FontSizeText;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* HighContrastCheckBox;

    // Motion and Effects
    UPROPERTY(meta = (BindWidget))
    UCheckBox* ReduceMotionCheckBox;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* FlashingLightsCheckBox;

    UPROPERTY(meta = (BindWidget))
    USlider* ScreenReaderSpeedSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScreenReaderSpeedText;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* ClosedCaptionsCheckBox;

    UPROPERTY(meta = (BindWidget))
    USlider* CaptionSizeSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CaptionSizeText;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* ScreenFlashReductionCheckBox;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* AudioCuesCheckBox;

    UPROPERTY(meta = (BindWidget))
    USlider* UIContrastSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* UIContrastText;

private:
    void InitializeColorBlindSettings();
    void InitializeFontSettings();
    void InitializeMotionSettings();
    void RefreshAdditionalAccessibilitySettings();

	//Apply all accessibility settings
    void ApplyAccessibilitySettings();
	void ApplyColorBlindFilter(int32 ColorBlindType);
	void ApplyFontSizeToAllUI(float FontSize);
	void ApplyHighContrastTheme(bool bHighContrast);
	void ApplyMotionReduction(bool bReduceMotion);
	void ApplyFlashingLightsReduction(bool bReduceFlashing);
	void ApplyScreenReaderSpeed(float Speed);
	void ApplyClosedCaptions(bool bEnabled);
	void ApplyCaptionSize(float Size);
	void ApplyScreenFlashReduction(bool bEnabled);
	void ApplyAudioCues(bool bEnabled);
	void ApplyUIContrast(float Contrast);

    UFUNCTION()
    void OnClosedCaptionsChanged(bool bIsChecked);

    UFUNCTION()
    void OnCaptionSizeChanged(float Value);

    UFUNCTION()
    void OnScreenFlashReductionChanged(bool bIsChecked);

    UFUNCTION()
    void OnAudioCuesChanged(bool bIsChecked);

    UFUNCTION()
    void OnUIContrastChanged(float Value);

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