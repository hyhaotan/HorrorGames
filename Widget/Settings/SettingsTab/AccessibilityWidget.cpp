#include "AccessibilityWidget.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "HorrorGame/Widget/Settings/SelectionWidget.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"

namespace
{
    const FString ColorBlindTypes[] = {
        TEXT("None"),
        TEXT("Protanopia (Red-Blind)"),
        TEXT("Deuteranopia (Green-Blind)"),
        TEXT("Tritanopia (Blue-Blind)")
    };
}

void UAccessibilityWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get settings instance with null check
    HorrorGameSettings = UHorrorGameSettings::Get();
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get HorrorGameSettings instance"));
        return;
    }

    // Initialize settings with null checks
    InitializeColorBlindSettings();
    InitializeFontSettings();
    InitializeMotionSettings();
}

UWidget* UAccessibilityWidget::NativeGetDesiredFocusTarget() const
{
    // Return first valid widget for focus
    if (ColorBlindSupportCheckBox && ColorBlindSupportCheckBox->IsValidLowLevel())
    {
        return ColorBlindSupportCheckBox;
    }

    if (FontSizeSlider && FontSizeSlider->IsValidLowLevel())
    {
        return FontSizeSlider;
    }

    return Super::NativeGetDesiredFocusTarget();
}

void UAccessibilityWidget::InitializeColorBlindSettings()
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in InitializeColorBlindSettings"));
        return;
    }

    // Color Blind Support CheckBox
    if (ColorBlindSupportCheckBox && ColorBlindSupportCheckBox->IsValidLowLevel())
    {
        ColorBlindSupportCheckBox->SetIsChecked(HorrorGameSettings->bColorBlindSupport);
        ColorBlindSupportCheckBox->OnCheckStateChanged.Clear();
        ColorBlindSupportCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnColorBlindSupportChanged);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ColorBlindSupportCheckBox is null or invalid"));
    }

    // Color Blind Type Selection
    if (ColorBlindTypeSelection && ColorBlindTypeSelection->IsValidLowLevel())
    {
        ColorBlindTypeSelection->Clear();
        const int32 CurrentColorBlindType = HorrorGameSettings->ColorBlindType;

        for (int32 i = 0; i < 4; ++i)
        {
            ColorBlindTypeSelection->AddOption({
                FText::FromString(ColorBlindTypes[i])
                });

            if (CurrentColorBlindType == i)
            {
                ColorBlindTypeSelection->SetCurrentSelection(i);
            }
        }

        ColorBlindTypeSelection->OnSelectionChange.BindLambda([this](int32 InSelection)
            {
                if (HorrorGameSettings)
                {
                    HorrorGameSettings->ColorBlindType = InSelection;
                    HorrorGameSettings->SaveConfig();
                    ApplyColorBlindFilter(InSelection);
                }
            });

        // Enable/disable based on support setting
        ColorBlindTypeSelection->SetIsEnabled(HorrorGameSettings->bColorBlindSupport);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ColorBlindTypeSelection is null or invalid"));
    }
}

void UAccessibilityWidget::InitializeFontSettings()
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in InitializeFontSettings"));
        return;
    }

    // Font Size Slider
    if (FontSizeSlider && FontSizeSlider->IsValidLowLevel())
    {
        FontSizeSlider->SetValue(HorrorGameSettings->FontSize);
        FontSizeSlider->OnValueChanged.Clear();
        FontSizeSlider->OnValueChanged.AddDynamic(this, &UAccessibilityWidget::OnFontSizeChanged);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FontSizeSlider is null or invalid"));
    }

    // Font Size Text
    if (FontSizeText && FontSizeText->IsValidLowLevel())
    {
        FontSizeText->SetText(FText::AsPercent(HorrorGameSettings->FontSize));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FontSizeText is null or invalid"));
    }

    // High Contrast CheckBox
    if (HighContrastCheckBox && HighContrastCheckBox->IsValidLowLevel())
    {
        HighContrastCheckBox->SetIsChecked(HorrorGameSettings->bHighContrastMode);
        HighContrastCheckBox->OnCheckStateChanged.Clear();
        HighContrastCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnHighContrastChanged);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HighContrastCheckBox is null or invalid"));
    }
}

void UAccessibilityWidget::InitializeMotionSettings()
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in InitializeMotionSettings"));
        return;
    }

    // Reduce Motion CheckBox
    if (ReduceMotionCheckBox && ReduceMotionCheckBox->IsValidLowLevel())
    {
        ReduceMotionCheckBox->SetIsChecked(HorrorGameSettings->bReduceMotion);
        ReduceMotionCheckBox->OnCheckStateChanged.Clear();
        ReduceMotionCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnReduceMotionChanged);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ReduceMotionCheckBox is null or invalid"));
    }
}

void UAccessibilityWidget::OnColorBlindSupportChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnColorBlindSupportChanged"));
        return;
    }

    HorrorGameSettings->bColorBlindSupport = bIsChecked;
    HorrorGameSettings->SaveConfig();

    // Enable/disable color blind type selection
    if (ColorBlindTypeSelection && ColorBlindTypeSelection->IsValidLowLevel())
    {
        ColorBlindTypeSelection->SetIsEnabled(bIsChecked);
    }

    // Apply or remove color blind filter
    if (bIsChecked)
    {
        ApplyColorBlindFilter(HorrorGameSettings->ColorBlindType);
    }
    else
    {
        ApplyColorBlindFilter(0); // None
    }
}

void UAccessibilityWidget::OnFontSizeChanged(float Value)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnFontSizeChanged"));
        return;
    }

    HorrorGameSettings->FontSize = Value;
    HorrorGameSettings->SaveConfig();

    if (FontSizeText && FontSizeText->IsValidLowLevel())
    {
        FontSizeText->SetText(FText::AsPercent(Value));
    }

    // Apply font size changes immediately to all UI elements
    ApplyFontSizeToAllUI(Value);
}

void UAccessibilityWidget::OnHighContrastChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnHighContrastChanged"));
        return;
    }

    HorrorGameSettings->bHighContrastMode = bIsChecked;
    HorrorGameSettings->SaveConfig();

    // Apply high contrast theme
    ApplyHighContrastTheme(bIsChecked);
}

void UAccessibilityWidget::OnReduceMotionChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnReduceMotionChanged"));
        return;
    }

    HorrorGameSettings->bReduceMotion = bIsChecked;
    HorrorGameSettings->SaveConfig();

    // Apply motion reduction settings
    ApplyMotionReduction(bIsChecked);
}

void UAccessibilityWidget::OnFlashingLightsChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnFlashingLightsChanged"));
        return;
    }

    HorrorGameSettings->bFlashingLightsReduction = bIsChecked;
    HorrorGameSettings->SaveConfig();

    // Apply flashing lights reduction
    ApplyFlashingLightsReduction(bIsChecked);
}

void UAccessibilityWidget::OnScreenReaderSpeedChanged(float Value)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnScreenReaderSpeedChanged"));
        return;
    }

    HorrorGameSettings->ScreenReaderSpeed = Value;
    HorrorGameSettings->SaveConfig();

    if (ScreenReaderSpeedText && ScreenReaderSpeedText->IsValidLowLevel())
    {
        ScreenReaderSpeedText->SetText(FText::AsPercent(Value));
    }

    // Apply screen reader speed change
    ApplyScreenReaderSpeed(Value);
}

void UAccessibilityWidget::OnClosedCaptionsChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnClosedCaptionsChanged"));
        return;
    }

    HorrorGameSettings->bClosedCaptionsEnabled = bIsChecked;
    HorrorGameSettings->SaveConfig();

    // Enable/disable caption size slider
    if (CaptionSizeSlider && CaptionSizeSlider->IsValidLowLevel())
    {
        CaptionSizeSlider->SetIsEnabled(bIsChecked);
    }

    // Apply closed captions setting
    ApplyClosedCaptions(bIsChecked);
}

void UAccessibilityWidget::OnCaptionSizeChanged(float Value)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnCaptionSizeChanged"));
        return;
    }

    HorrorGameSettings->CaptionSize = Value;
    HorrorGameSettings->SaveConfig();

    if (CaptionSizeText && CaptionSizeText->IsValidLowLevel())
    {
        CaptionSizeText->SetText(FText::AsPercent(Value));
    }

    // Apply caption size change
    ApplyCaptionSize(Value);
}

void UAccessibilityWidget::OnScreenFlashReductionChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnScreenFlashReductionChanged"));
        return;
    }

    HorrorGameSettings->bScreenFlashReduction = bIsChecked;
    HorrorGameSettings->SaveConfig();

    // Apply screen flash reduction
    ApplyScreenFlashReduction(bIsChecked);
}

void UAccessibilityWidget::OnAudioCuesChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnAudioCuesChanged"));
        return;
    }

    HorrorGameSettings->bAudioCuesEnabled = bIsChecked;
    HorrorGameSettings->SaveConfig();

    // Apply audio cues setting
    ApplyAudioCues(bIsChecked);
}

void UAccessibilityWidget::OnUIContrastChanged(float Value)
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in OnUIContrastChanged"));
        return;
    }

    HorrorGameSettings->UIContrast = Value;
    HorrorGameSettings->SaveConfig();

    if (UIContrastText && UIContrastText->IsValidLowLevel())
    {
        UIContrastText->SetText(FText::AsPercent(Value));
    }

    // Apply UI contrast change
    ApplyUIContrast(Value);
}

void UAccessibilityWidget::RefreshValues()
{
    // Get settings instance if not available
    if (!HorrorGameSettings)
    {
        HorrorGameSettings = UHorrorGameSettings::Get();
    }

    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot refresh values: HorrorGameSettings is null"));
        return;
    }

    // Refresh Color Blind Settings
    if (ColorBlindSupportCheckBox && ColorBlindSupportCheckBox->IsValidLowLevel())
    {
        ColorBlindSupportCheckBox->SetIsChecked(HorrorGameSettings->bColorBlindSupport);
        ColorBlindSupportCheckBox->OnCheckStateChanged.Clear();
        ColorBlindSupportCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnColorBlindSupportChanged);
    }

    // Refresh Color Blind Type Selection
    if (ColorBlindTypeSelection && ColorBlindTypeSelection->IsValidLowLevel())
    {
        ColorBlindTypeSelection->Clear();
        const int32 CurrentColorBlindType = HorrorGameSettings->ColorBlindType;

        for (int32 i = 0; i < 4; ++i)
        {
            ColorBlindTypeSelection->AddOption({
                FText::FromString(ColorBlindTypes[i])
                });

            if (CurrentColorBlindType == i)
            {
                ColorBlindTypeSelection->SetCurrentSelection(i);
            }
        }

        ColorBlindTypeSelection->OnSelectionChange.BindLambda([this](int32 InSelection)
            {
                if (HorrorGameSettings)
                {
                    HorrorGameSettings->ColorBlindType = InSelection;
                    HorrorGameSettings->SaveConfig();
                    ApplyColorBlindFilter(InSelection);
                }
            });

        // Enable/disable based on support checkbox
        ColorBlindTypeSelection->SetIsEnabled(HorrorGameSettings->bColorBlindSupport);
    }

    // Refresh Font Settings
    if (FontSizeSlider && FontSizeSlider->IsValidLowLevel() && FontSizeText && FontSizeText->IsValidLowLevel())
    {
        float FontSize = HorrorGameSettings->FontSize;
        FontSizeSlider->SetValue(FontSize);
        FontSizeText->SetText(FText::AsPercent(FontSize));
        FontSizeSlider->OnValueChanged.Clear();
        FontSizeSlider->OnValueChanged.AddDynamic(this, &UAccessibilityWidget::OnFontSizeChanged);
    }

    if (HighContrastCheckBox && HighContrastCheckBox->IsValidLowLevel())
    {
        HighContrastCheckBox->SetIsChecked(HorrorGameSettings->bHighContrastMode);
        HighContrastCheckBox->OnCheckStateChanged.Clear();
        HighContrastCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnHighContrastChanged);
    }

    // Refresh Motion Settings
    if (ReduceMotionCheckBox && ReduceMotionCheckBox->IsValidLowLevel())
    {
        ReduceMotionCheckBox->SetIsChecked(HorrorGameSettings->bReduceMotion);
        ReduceMotionCheckBox->OnCheckStateChanged.Clear();
        ReduceMotionCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnReduceMotionChanged);
    }

    // Refresh Additional Settings
    RefreshAdditionalAccessibilitySettings();
}

void UAccessibilityWidget::ResetToDefaults()
{
    if (!HorrorGameSettings)
    {
        HorrorGameSettings = UHorrorGameSettings::Get();
    }

    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot reset to defaults: HorrorGameSettings is null"));
        return;
    }

    // Reset all accessibility settings to defaults
    HorrorGameSettings->bColorBlindSupport = false;
    HorrorGameSettings->ColorBlindType = 0; // None
    HorrorGameSettings->FontSize = 1.0f; // 100%
    HorrorGameSettings->bHighContrastMode = false;
    HorrorGameSettings->bReduceMotion = false;

    // Reset additional settings
    HorrorGameSettings->bFlashingLightsReduction = false;
    HorrorGameSettings->ScreenReaderSpeed = 1.0f;
    HorrorGameSettings->bClosedCaptionsEnabled = false;
    HorrorGameSettings->CaptionSize = 1.0f;
    HorrorGameSettings->bScreenFlashReduction = false;
    HorrorGameSettings->bAudioCuesEnabled = false;
    HorrorGameSettings->UIContrast = 1.0f;

    // Save changes
    HorrorGameSettings->SaveConfig();

    // Refresh UI to show new values
    RefreshValues();

    // Apply default settings
    ApplyAccessibilitySettings();

    UE_LOG(LogTemp, Log, TEXT("Accessibility settings reset to defaults"));
}

void UAccessibilityWidget::RefreshAdditionalAccessibilitySettings()
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in RefreshAdditionalAccessibilitySettings"));
        return;
    }

    // Flashing Lights Reduction
    if (FlashingLightsCheckBox && FlashingLightsCheckBox->IsValidLowLevel())
    {
        FlashingLightsCheckBox->SetIsChecked(HorrorGameSettings->bFlashingLightsReduction);
        FlashingLightsCheckBox->OnCheckStateChanged.Clear();
        FlashingLightsCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnFlashingLightsChanged);
    }

    // Screen Reader Speed
    if (ScreenReaderSpeedSlider && ScreenReaderSpeedSlider->IsValidLowLevel() &&
        ScreenReaderSpeedText && ScreenReaderSpeedText->IsValidLowLevel())
    {
        float ScreenReaderSpeed = HorrorGameSettings->ScreenReaderSpeed;
        ScreenReaderSpeedSlider->SetValue(ScreenReaderSpeed);
        ScreenReaderSpeedText->SetText(FText::AsPercent(ScreenReaderSpeed));
        ScreenReaderSpeedSlider->OnValueChanged.Clear();
        ScreenReaderSpeedSlider->OnValueChanged.AddDynamic(this, &UAccessibilityWidget::OnScreenReaderSpeedChanged);
    }

    // Closed Captions
    if (ClosedCaptionsCheckBox && ClosedCaptionsCheckBox->IsValidLowLevel())
    {
        ClosedCaptionsCheckBox->SetIsChecked(HorrorGameSettings->bClosedCaptionsEnabled);
        ClosedCaptionsCheckBox->OnCheckStateChanged.Clear();
        ClosedCaptionsCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnClosedCaptionsChanged);
    }

    // Caption Size
    if (CaptionSizeSlider && CaptionSizeSlider->IsValidLowLevel() &&
        CaptionSizeText && CaptionSizeText->IsValidLowLevel())
    {
        float CaptionSize = HorrorGameSettings->CaptionSize;
        CaptionSizeSlider->SetValue(CaptionSize);
        CaptionSizeText->SetText(FText::AsPercent(CaptionSize));
        CaptionSizeSlider->OnValueChanged.Clear();
        CaptionSizeSlider->OnValueChanged.AddDynamic(this, &UAccessibilityWidget::OnCaptionSizeChanged);

        // Enable/disable based on captions checkbox
        CaptionSizeSlider->SetIsEnabled(HorrorGameSettings->bClosedCaptionsEnabled);
    }

    // Screen Flash Reduction
    if (ScreenFlashReductionCheckBox && ScreenFlashReductionCheckBox->IsValidLowLevel())
    {
        ScreenFlashReductionCheckBox->SetIsChecked(HorrorGameSettings->bScreenFlashReduction);
        ScreenFlashReductionCheckBox->OnCheckStateChanged.Clear();
        ScreenFlashReductionCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnScreenFlashReductionChanged);
    }

    // Audio Cues
    if (AudioCuesCheckBox && AudioCuesCheckBox->IsValidLowLevel())
    {
        AudioCuesCheckBox->SetIsChecked(HorrorGameSettings->bAudioCuesEnabled);
        AudioCuesCheckBox->OnCheckStateChanged.Clear();
        AudioCuesCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnAudioCuesChanged);
    }

    // UI Contrast
    if (UIContrastSlider && UIContrastSlider->IsValidLowLevel() &&
        UIContrastText && UIContrastText->IsValidLowLevel())
    {
        float UIContrast = HorrorGameSettings->UIContrast;
        UIContrastSlider->SetValue(UIContrast);
        UIContrastText->SetText(FText::AsPercent(UIContrast));
        UIContrastSlider->OnValueChanged.Clear();
        UIContrastSlider->OnValueChanged.AddDynamic(this, &UAccessibilityWidget::OnUIContrastChanged);
    }
}

void UAccessibilityWidget::ApplyAccessibilitySettings()
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings is null in ApplyAccessibilitySettings"));
        return;
    }

    ApplyColorBlindFilter(HorrorGameSettings->bColorBlindSupport ? HorrorGameSettings->ColorBlindType : 0);
    ApplyFontSizeToAllUI(HorrorGameSettings->FontSize);
    ApplyHighContrastTheme(HorrorGameSettings->bHighContrastMode);
    ApplyMotionReduction(HorrorGameSettings->bReduceMotion);
    ApplyFlashingLightsReduction(HorrorGameSettings->bFlashingLightsReduction);
    ApplyClosedCaptions(HorrorGameSettings->bClosedCaptionsEnabled);
    ApplyCaptionSize(HorrorGameSettings->CaptionSize);
    ApplyScreenFlashReduction(HorrorGameSettings->bScreenFlashReduction);
    ApplyAudioCues(HorrorGameSettings->bAudioCuesEnabled);
    ApplyUIContrast(HorrorGameSettings->UIContrast);
}

void UAccessibilityWidget::ApplyColorBlindFilter(int32 ColorBlindType)
{
    // Implementation would depend on your post-process system
    // Example: Set post-process material parameters based on color blind type

    switch (ColorBlindType)
    {
    case 0: // None
        // Remove color blind filter
        break;
    case 1: // Protanopia (Red-Blind)
        // Apply red-blind correction filter
        break;
    case 2: // Deuteranopia (Green-Blind)
        // Apply green-blind correction filter
        break;
    case 3: // Tritanopia (Blue-Blind)
        // Apply blue-blind correction filter
        break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("Invalid color blind type: %d"), ColorBlindType);
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("Applied color blind filter type: %d"), ColorBlindType);
}

void UAccessibilityWidget::ApplyFontSizeToAllUI(float FontSizeMultiplier)
{
    // Validate input
    FontSizeMultiplier = FMath::Clamp(FontSizeMultiplier, 0.5f, 3.0f);

    // Broadcast font size change to all UI elements
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnFontSizeChanged.Broadcast(FontSizeMultiplier);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied font size multiplier: %f"), FontSizeMultiplier);
}

void UAccessibilityWidget::ApplyHighContrastTheme(bool bHighContrast)
{
    // Switch UI theme to high contrast version
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnHighContrastChanged.Broadcast(bHighContrast);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied high contrast mode: %s"), bHighContrast ? TEXT("On") : TEXT("Off"));
}

void UAccessibilityWidget::ApplyMotionReduction(bool bReduceMotion)
{
    // Reduce or disable animations based on setting
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnMotionReductionChanged.Broadcast(bReduceMotion);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied motion reduction: %s"), bReduceMotion ? TEXT("On") : TEXT("Off"));
}

void UAccessibilityWidget::ApplyFlashingLightsReduction(bool bReduceFlashing)
{
    // Reduce intensity of flashing effects
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnFlashingLightsReductionChanged.Broadcast(bReduceFlashing);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied flashing lights reduction: %s"), bReduceFlashing ? TEXT("On") : TEXT("Off"));
}

void UAccessibilityWidget::ApplyClosedCaptions(bool bEnabled)
{
    // Enable/disable closed captions system
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnClosedCaptionsChanged.Broadcast(bEnabled);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied closed captions: %s"), bEnabled ? TEXT("On") : TEXT("Off"));
}

void UAccessibilityWidget::ApplyCaptionSize(float SizeMultiplier)
{
    // Validate input
    SizeMultiplier = FMath::Clamp(SizeMultiplier, 0.5f, 3.0f);

    // Apply caption size multiplier
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnCaptionSizeChanged.Broadcast(SizeMultiplier);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied caption size: %f"), SizeMultiplier);
}

void UAccessibilityWidget::ApplyScreenFlashReduction(bool bReduceFlash)
{
    // Reduce screen flash intensity
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnScreenFlashReductionChanged.Broadcast(bReduceFlash);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied screen flash reduction: %s"), bReduceFlash ? TEXT("On") : TEXT("Off"));
}

void UAccessibilityWidget::ApplyAudioCues(bool bEnabled)
{
    // Enable/disable audio cues for visual elements
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnAudioCuesChanged.Broadcast(bEnabled);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied audio cues: %s"), bEnabled ? TEXT("On") : TEXT("Off"));
}

void UAccessibilityWidget::ApplyUIContrast(float ContrastMultiplier)
{
    // Validate input
    ContrastMultiplier = FMath::Clamp(ContrastMultiplier, 0.5f, 3.0f);

    // Apply UI contrast multiplier
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnUIContrastChanged.Broadcast(ContrastMultiplier);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied UI contrast: %f"), ContrastMultiplier);
}

void UAccessibilityWidget::ApplyScreenReaderSpeed(float SpeedMultiplier)
{
    // Validate input
    SpeedMultiplier = FMath::Clamp(SpeedMultiplier, 0.5f, 3.0f);

    // Apply screen reader speed
    if (HorrorGameSettings)
    {
        HorrorGameSettings->OnScreenReaderSpeedChanged.Broadcast(SpeedMultiplier);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied screen reader speed: %f"), SpeedMultiplier);
}