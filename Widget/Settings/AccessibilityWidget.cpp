#include "HorrorGame/Widget/Settings/AccessibilityWidget.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "SelectionWidget.h"
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
    HorrorGameSettings = UHorrorGameSettings::Get();

    InitializeColorBlindSettings();
    InitializeFontSettings();
    InitializeMotionSettings();
}

UWidget* UAccessibilityWidget::NativeGetDesiredFocusTarget() const
{
    return ColorBlindSupportCheckBox;
}

void UAccessibilityWidget::InitializeColorBlindSettings()
{
    // Color Blind Support
    ColorBlindSupportCheckBox->SetIsChecked(HorrorGameSettings->bColorBlindSupport);
    ColorBlindSupportCheckBox->OnCheckStateChanged.Clear();
    ColorBlindSupportCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnColorBlindSupportChanged);

    // Color Blind Type
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
            HorrorGameSettings->ColorBlindType = InSelection;
            HorrorGameSettings->SaveConfig();
            // Apply color blind filter
            // TODO: Implement color blind filter application
        });
}

void UAccessibilityWidget::InitializeFontSettings()
{
    // Font Size
    FontSizeSlider->SetValue(HorrorGameSettings->FontSize);
    FontSizeText->SetText(FText::AsPercent(HorrorGameSettings->FontSize));
    FontSizeSlider->OnValueChanged.Clear();
    FontSizeSlider->OnValueChanged.AddDynamic(this, &UAccessibilityWidget::OnFontSizeChanged);

    // High Contrast
    HighContrastCheckBox->SetIsChecked(HorrorGameSettings->bHighContrastMode);
    HighContrastCheckBox->OnCheckStateChanged.Clear();
    HighContrastCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnHighContrastChanged);
}

void UAccessibilityWidget::InitializeMotionSettings()
{
    // Reduce Motion
    ReduceMotionCheckBox->SetIsChecked(HorrorGameSettings->bReduceMotion);
    ReduceMotionCheckBox->OnCheckStateChanged.Clear();
    ReduceMotionCheckBox->OnCheckStateChanged.AddDynamic(this, &UAccessibilityWidget::OnReduceMotionChanged);
}

void UAccessibilityWidget::OnColorBlindSupportChanged(bool bIsChecked)
{
    HorrorGameSettings->bColorBlindSupport = bIsChecked;
    HorrorGameSettings->SaveConfig();

    // Enable/disable color blind type selection
    ColorBlindTypeSelection->SetIsEnabled(bIsChecked);
}

void UAccessibilityWidget::OnFontSizeChanged(float Value)
{
    HorrorGameSettings->FontSize = Value;
    HorrorGameSettings->SaveConfig();
    FontSizeText->SetText(FText::AsPercent(Value));

    // Apply font size changes immediately
    // TODO: Implement font size application to all UI elements
}

void UAccessibilityWidget::OnHighContrastChanged(bool bIsChecked)
{
    HorrorGameSettings->bHighContrastMode = bIsChecked;
    HorrorGameSettings->SaveConfig();

    // Apply high contrast mode
    // TODO: Implement high contrast theme switching
}

void UAccessibilityWidget::OnReduceMotionChanged(bool bIsChecked)
{
    HorrorGameSettings->bReduceMotion = bIsChecked;
    HorrorGameSettings->SaveConfig();
}

void UAccessibilityWidget::OnFlashingLightsChanged(bool bIsChecked)
{
    // Implementation for flashing lights setting
}

void UAccessibilityWidget::OnScreenReaderSpeedChanged(float Value)
{
    // Implementation for screen reader speed
}