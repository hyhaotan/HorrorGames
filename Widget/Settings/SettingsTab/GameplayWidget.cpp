#include "GameplayWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "HorrorGame/Widget/Settings/SelectionWidget.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

namespace
{
    // Difficulty constants
    constexpr int32 DifficultyOptions[] = { 0, 1, 2, 3 }; // Easy, Normal, Hard, Nightmare
    constexpr int32 DifficultyCount = UE_ARRAY_COUNT(DifficultyOptions);

    const FString DifficultyNames[] = {
        TEXT("Easy"),
        TEXT("Normal"),
        TEXT("Hard"),
        TEXT("Nightmare")
    };

    // Language constants
    const FString LanguageOptions[] = {
        TEXT("English"),
        TEXT("Tiếng Việt"),
        TEXT("Français"),
        TEXT("Deutsch"),
        TEXT("Español"),
        TEXT("日本語"),
        TEXT("한국어"),
        TEXT("中文")
    };

    const FString LanguageCodes[] = {
        TEXT("en"),
        TEXT("vi"),
        TEXT("fr"),
        TEXT("de"),
        TEXT("es"),
        TEXT("ja"),
        TEXT("ko"),
        TEXT("zh")
    };

    constexpr int32 LanguageCount = UE_ARRAY_COUNT(LanguageOptions);

    // Ensure arrays have same size
    static_assert(UE_ARRAY_COUNT(DifficultyNames) == DifficultyCount, "DifficultyNames array size mismatch");
    static_assert(UE_ARRAY_COUNT(LanguageOptions) == UE_ARRAY_COUNT(LanguageCodes), "Language arrays size mismatch");
}

UGameplayWidget::UGameplayWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , GameUserSettings(nullptr)
    , HorrorGameSettings(nullptr)
{
}

void UGameplayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize settings references
    GameUserSettings = UGameUserSettings::GetGameUserSettings();
    HorrorGameSettings = UHorrorGameSettings::Get();

    // Debug logging
    UE_LOG(LogTemp, Warning, TEXT("GameplayWidget::NativeConstruct"));
    UE_LOG(LogTemp, Warning, TEXT("DifficultySelection: %s"),
        DifficultySelection ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("LanguageSelection: %s"),
        LanguageSelection ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("HorrorGameSettings: %s"),
        HorrorGameSettings ? TEXT("Valid") : TEXT("NULL"));

    // Validate critical components
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get HorrorGameSettings instance!"));
        return;
    }

    // Initialize all components
    RefreshValues();
    InitializeDifficulty();
    InitializeLanguage();
    InitializeGameplayOptions();
}

UWidget* UGameplayWidget::NativeGetDesiredFocusTarget() const
{
    return DifficultySelection ? DifficultySelection : Super::NativeGetDesiredFocusTarget();
}

void UGameplayWidget::InitializeLanguage()
{
    if (!LanguageSelection)
    {
        UE_LOG(LogTemp, Error, TEXT("LanguageSelection widget is null"));
        return;
    }
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("HorrorGameSettings is null"));
        return;
    }

    LanguageSelection->Clear();
    const FString CurrentLanguage = HorrorGameSettings->GetLanguage();
    int32 FoundIndex = -1;

    // Add language options
    for (int32 i = 0; i < LanguageCount; ++i)
    {
        LanguageSelection->AddOption({
            FText::FromString(LanguageOptions[i])
            });

        if (CurrentLanguage == LanguageCodes[i])
        {
            FoundIndex = i;
        }
    }

    // Set current selection only if we found a match and have options
    if (FoundIndex >= 0 && LanguageCount > 0)
    {
        LanguageSelection->SetCurrentSelection(FoundIndex);
    }
    else if (LanguageCount > 0)
    {
        // Default to first option if current language not found
        LanguageSelection->SetCurrentSelection(0);
        UE_LOG(LogTemp, Warning, TEXT("Current language not found, defaulting to first option"));
    }

    // Bind selection change event
    LanguageSelection->OnSelectionChange.BindLambda([this](int32 InSelection)
        {
            if (HorrorGameSettings && InSelection >= 0 && InSelection < LanguageCount)
            {
                HorrorGameSettings->SetLanguage(LanguageCodes[InSelection]);
                HorrorGameSettings->SaveConfig();
                // Notify game to reload localization
                HorrorGameSettings->OnLanguageChanged.Broadcast(LanguageCodes[InSelection]);
                UE_LOG(LogTemp, Log, TEXT("Language changed to: %s"), *LanguageCodes[InSelection]);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Invalid language selection: %d"), InSelection);
            }
        });
}

void UGameplayWidget::InitializeDifficulty()
{
    if (!DifficultySelection)
    {
        UE_LOG(LogTemp, Error, TEXT("DifficultySelection widget is null"));
        return;
    }
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("HorrorGameSettings is null"));
        return;
    }

    DifficultySelection->Clear();
    const int32 CurrentDifficulty = HorrorGameSettings->GetDifficulty();
    int32 FoundIndex = -1;

    // Add difficulty options
    for (int32 i = 0; i < DifficultyCount; ++i)
    {
        DifficultySelection->AddOption({
            FText::FromString(DifficultyNames[i])
            });

        if (CurrentDifficulty == DifficultyOptions[i])
        {
            FoundIndex = i;
        }
    }

    // Set current selection only if we found a match and have options
    if (FoundIndex >= 0 && DifficultyCount > 0)
    {
        DifficultySelection->SetCurrentSelection(FoundIndex);
    }
    else if (DifficultyCount > 0)
    {
        // Default to first option if current difficulty not found
        DifficultySelection->SetCurrentSelection(0);
        UE_LOG(LogTemp, Warning, TEXT("Current difficulty not found, defaulting to first option"));
    }

    // Bind selection change event
    DifficultySelection->OnSelectionChange.BindLambda([this](int32 InSelection)
        {
            if (HorrorGameSettings && InSelection >= 0 && InSelection < DifficultyCount)
            {
                HorrorGameSettings->SetDifficulty(DifficultyOptions[InSelection]);
                HorrorGameSettings->SaveConfig();
                UE_LOG(LogTemp, Log, TEXT("Difficulty changed to: %d"), DifficultyOptions[InSelection]);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Invalid difficulty selection: %d"), InSelection);
            }
        });
}

void UGameplayWidget::InitializeGameplayOptions()
{
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("HorrorGameSettings is null in InitializeGameplayOptions"));
        return;
    }

    // Initialize Subtitles
    if (SubtitlesCheckBox)
    {
        SubtitlesCheckBox->SetIsChecked(HorrorGameSettings->GetSubtitlesEnabled());
        SubtitlesCheckBox->OnCheckStateChanged.Clear();
        SubtitlesCheckBox->OnCheckStateChanged.AddDynamic(this, &UGameplayWidget::OnSubtitlesChanged);
    }

    // Initialize Auto Save
    if (AutoSaveCheckBox)
    {
        AutoSaveCheckBox->SetIsChecked(HorrorGameSettings->GetAutoSaveEnabled());
        AutoSaveCheckBox->OnCheckStateChanged.Clear();
        AutoSaveCheckBox->OnCheckStateChanged.AddDynamic(this, &UGameplayWidget::OnAutoSaveChanged);
    }

    // Initialize UI Scale
    if (UIScaleSlider)
    {
        const float UIScale = HorrorGameSettings->GetUIScale();
        UIScaleSlider->SetValue(UIScale);
        UIScaleSlider->OnValueChanged.Clear();
        UIScaleSlider->OnValueChanged.AddDynamic(this, &UGameplayWidget::OnUIScaleChanged);

        if (UIScaleValueText)
        {
            UIScaleValueText->SetText(FText::AsPercent(UIScale));
        }
    }

    // Initialize Tutorial Hints
    if (TutorialHintsCheckBox)
    {
        TutorialHintsCheckBox->SetIsChecked(HorrorGameSettings->GetTutorialHintsEnabled());
        TutorialHintsCheckBox->OnCheckStateChanged.Clear();
        TutorialHintsCheckBox->OnCheckStateChanged.AddDynamic(this, &UGameplayWidget::OnTutorialHintsChanged);
    }

    // Initialize Camera Shake
    if (CameraShakeCheckBox)
    {
        CameraShakeCheckBox->SetIsChecked(HorrorGameSettings->GetCameraShakeEnabled());
        CameraShakeCheckBox->OnCheckStateChanged.Clear();
        CameraShakeCheckBox->OnCheckStateChanged.AddDynamic(this, &UGameplayWidget::OnCameraShakeChanged);
    }

    // Initialize Interaction Hold Time
    if (InteractionHoldTimeSlider)
    {
        const float HoldTime = HorrorGameSettings->GetInteractionHoldTime();
        InteractionHoldTimeSlider->SetValue(HoldTime);
        InteractionHoldTimeSlider->OnValueChanged.Clear();
        InteractionHoldTimeSlider->OnValueChanged.AddDynamic(this, &UGameplayWidget::OnInteractionHoldTimeChanged);

        if (InteractionHoldTimeText)
        {
            InteractionHoldTimeText->SetText(FText::AsNumber(HoldTime, &FNumberFormattingOptions::DefaultNoGrouping()));
        }
    }
}

// Event Handlers
void UGameplayWidget::OnSubtitlesChanged(bool bIsChecked)
{
    if (HorrorGameSettings)
    {
        HorrorGameSettings->SetSubtitlesEnabled(bIsChecked);
        HorrorGameSettings->SaveConfig();
        UE_LOG(LogTemp, Log, TEXT("Subtitles changed to: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void UGameplayWidget::OnAutoSaveChanged(bool bIsChecked)
{
    if (HorrorGameSettings)
    {
        HorrorGameSettings->SetAutoSaveEnabled(bIsChecked);
        HorrorGameSettings->SaveConfig();
        UE_LOG(LogTemp, Log, TEXT("Auto Save changed to: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void UGameplayWidget::OnUIScaleChanged(float Value)
{
    if (HorrorGameSettings)
    {
        HorrorGameSettings->SetUIScale(Value);
        HorrorGameSettings->SaveConfig();

        if (UIScaleValueText)
        {
            UIScaleValueText->SetText(FText::AsPercent(Value));
        }

        // Apply UI scale immediately
        HorrorGameSettings->OnUIScaleChanged.Broadcast(Value);
        UE_LOG(LogTemp, Log, TEXT("UI Scale changed to: %f"), Value);
    }
}

void UGameplayWidget::OnTutorialHintsChanged(bool bIsChecked)
{
    if (HorrorGameSettings)
    {
        HorrorGameSettings->SetTutorialHintsEnabled(bIsChecked);
        HorrorGameSettings->SaveConfig();
        UE_LOG(LogTemp, Log, TEXT("Tutorial Hints changed to: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void UGameplayWidget::OnCameraShakeChanged(bool bIsChecked)
{
    if (HorrorGameSettings)
    {
        HorrorGameSettings->SetCameraShakeEnabled(bIsChecked);
        HorrorGameSettings->SaveConfig();
        UE_LOG(LogTemp, Log, TEXT("Camera Shake changed to: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void UGameplayWidget::OnInteractionHoldTimeChanged(float Value)
{
    if (HorrorGameSettings)
    {
        HorrorGameSettings->SetInteractionHoldTime(Value);
        HorrorGameSettings->SaveConfig();

        if (InteractionHoldTimeText)
        {
            InteractionHoldTimeText->SetText(FText::AsNumber(Value, &FNumberFormattingOptions::DefaultNoGrouping()));
        }

        UE_LOG(LogTemp, Log, TEXT("Interaction Hold Time changed to: %f"), Value);
    }
}

void UGameplayWidget::RefreshValues()
{
    if (!HorrorGameSettings)
    {
        HorrorGameSettings = UHorrorGameSettings::Get();
        if (!HorrorGameSettings)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot get HorrorGameSettings instance in RefreshValues"));
            return;
        }
    }

    // Refresh Difficulty Selection
    if (DifficultySelection)
    {
        const int32 CurrentDifficulty = HorrorGameSettings->GetDifficulty();
        for (int32 i = 0; i < DifficultyCount; ++i)
        {
            if (CurrentDifficulty == DifficultyOptions[i])
            {
                DifficultySelection->SetCurrentSelection(i);
                break;
            }
        }
    }

    // Refresh Language Selection
    if (LanguageSelection)
    {
        const FString CurrentLanguage = HorrorGameSettings->GetLanguage();
        for (int32 i = 0; i < LanguageCount; ++i)
        {
            if (CurrentLanguage == LanguageCodes[i])
            {
                LanguageSelection->SetCurrentSelection(i);
                break;
            }
        }
    }

    // Refresh Checkboxes
    if (SubtitlesCheckBox)
    {
        SubtitlesCheckBox->SetIsChecked(HorrorGameSettings->GetSubtitlesEnabled());
    }

    if (AutoSaveCheckBox)
    {
        AutoSaveCheckBox->SetIsChecked(HorrorGameSettings->GetAutoSaveEnabled());
    }

    if (TutorialHintsCheckBox)
    {
        TutorialHintsCheckBox->SetIsChecked(HorrorGameSettings->GetTutorialHintsEnabled());
    }

    if (CameraShakeCheckBox)
    {
        CameraShakeCheckBox->SetIsChecked(HorrorGameSettings->GetCameraShakeEnabled());
    }

    // Refresh Sliders
    if (UIScaleSlider)
    {
        const float UIScale = HorrorGameSettings->GetUIScale();
        UIScaleSlider->SetValue(UIScale);

        if (UIScaleValueText)
        {
            UIScaleValueText->SetText(FText::AsPercent(UIScale));
        }
    }

    if (InteractionHoldTimeSlider)
    {
        const float HoldTime = HorrorGameSettings->GetInteractionHoldTime();
        InteractionHoldTimeSlider->SetValue(HoldTime);

        if (InteractionHoldTimeText)
        {
            InteractionHoldTimeText->SetText(FText::AsNumber(HoldTime, &FNumberFormattingOptions::DefaultNoGrouping()));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("RefreshValues completed"));
}

void UGameplayWidget::ResetToDefaults()
{
    if (!HorrorGameSettings)
    {
        HorrorGameSettings = UHorrorGameSettings::Get();
        if (!HorrorGameSettings)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot get HorrorGameSettings instance in ResetToDefaults"));
            return;
        }
    }

    // Reset all gameplay settings to defaults
    HorrorGameSettings->SetDifficulty(1); // Normal difficulty
    HorrorGameSettings->SetLanguage(TEXT("en")); // English
    HorrorGameSettings->SetSubtitlesEnabled(true);
    HorrorGameSettings->SetAutoSaveEnabled(true);
    HorrorGameSettings->SetTutorialHintsEnabled(true);
    HorrorGameSettings->SetCameraShakeEnabled(true);
    HorrorGameSettings->SetUIScale(1.0f);
    HorrorGameSettings->SetInteractionHoldTime(0.5f);

    // Save changes
    HorrorGameSettings->SaveConfig();

    // Refresh UI to show new values
    RefreshValues();

    // Rebind selection widgets to ensure proper state
    InitializeDifficulty();
    InitializeLanguage();

    UE_LOG(LogTemp, Log, TEXT("Gameplay settings reset to defaults"));
}

void UGameplayWidget::BindAllEvents()
{
    // This function can be called to ensure all events are properly bound
    // Usually called after widget reconstruction or when needed

    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("HorrorGameSettings is null in BindAllEvents"));
        return;
    }

    // Rebind difficulty selection
    if (DifficultySelection)
    {
        DifficultySelection->OnSelectionChange.BindLambda([this](int32 InSelection)
            {
                if (HorrorGameSettings && InSelection >= 0 && InSelection < DifficultyCount)
                {
                    HorrorGameSettings->SetDifficulty(DifficultyOptions[InSelection]);
                    HorrorGameSettings->SaveConfig();
                }
            });
    }

    // Rebind language selection  
    if (LanguageSelection)
    {
        LanguageSelection->OnSelectionChange.BindLambda([this](int32 InSelection)
            {
                if (HorrorGameSettings && InSelection >= 0 && InSelection < LanguageCount)
                {
                    HorrorGameSettings->SetLanguage(LanguageCodes[InSelection]);
                    HorrorGameSettings->SaveConfig();
                    HorrorGameSettings->OnLanguageChanged.Broadcast(LanguageCodes[InSelection]);
                }
            });
    }

    UE_LOG(LogTemp, Log, TEXT("All events rebound successfully"));
}