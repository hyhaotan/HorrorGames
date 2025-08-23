#include "HorrorGame/Widget/Settings/GameplayWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "SelectionWidget.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    constexpr int32 DifficultyOptions[] = { 0, 1, 2, 3 }; // Easy, Normal, Hard, Nightmare
    const FString DifficultyNames[] = { TEXT("Easy"), TEXT("Normal"), TEXT("Hard"), TEXT("Nightmare") };

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
}

void UGameplayWidget::NativeConstruct()
{
    GameUserSettings = UGameUserSettings::GetGameUserSettings();
    HorrorGameSettings = UHorrorGameSettings::Get();

    InitializeDifficulty();
    InitializeLanguage();
    InitializeGameplayOptions();
}

UWidget* UGameplayWidget::NativeGetDesiredFocusTarget() const
{
    return DifficultySelection;
}

void UGameplayWidget::InitializeDifficulty()
{
    DifficultySelection->Clear();

    const int32 CurrentDifficulty = HorrorGameSettings->GetDifficulty();

    for (int32 i = 0; i < 4; ++i)
    {
        DifficultySelection->AddOption({
            FText::FromString(DifficultyNames[i])
            });

        if (CurrentDifficulty == DifficultyOptions[i])
        {
            DifficultySelection->SetCurrentSelection(i);
        }
    }

    DifficultySelection->OnSelectionChange.BindLambda([this](int32 InSelection)
        {
            HorrorGameSettings->SetDifficulty(DifficultyOptions[InSelection]);
            HorrorGameSettings->SaveConfig();
        });
}

void UGameplayWidget::InitializeLanguage()
{
    LanguageSelection->Clear();

    const FString CurrentLanguage = HorrorGameSettings->GetLanguage();

    for (int32 i = 0; i < 8; ++i)
    {
        LanguageSelection->AddOption({
            FText::FromString(LanguageOptions[i])
            });

        if (CurrentLanguage == LanguageCodes[i])
        {
            LanguageSelection->SetCurrentSelection(i);
        }
    }

    LanguageSelection->OnSelectionChange.BindLambda([this](int32 InSelection)
        {
            HorrorGameSettings->SetLanguage(LanguageCodes[InSelection]);
            HorrorGameSettings->SaveConfig();
            // Notify game to reload localization
            HorrorGameSettings->OnLanguageChanged.Broadcast(LanguageCodes[InSelection]);
        });
}

void UGameplayWidget::InitializeGameplayOptions()
{
    // Subtitles
    SubtitlesCheckBox->SetIsChecked(HorrorGameSettings->GetSubtitlesEnabled());
    SubtitlesCheckBox->OnCheckStateChanged.Clear();
    SubtitlesCheckBox->OnCheckStateChanged.AddDynamic(this, &UGameplayWidget::OnSubtitlesChanged);

    // Auto Save
    AutoSaveCheckBox->SetIsChecked(HorrorGameSettings->GetAutoSaveEnabled());
    AutoSaveCheckBox->OnCheckStateChanged.Clear();
    AutoSaveCheckBox->OnCheckStateChanged.AddDynamic(this, &UGameplayWidget::OnAutoSaveChanged);

    // UI Scale
    UIScaleSlider->SetValue(HorrorGameSettings->GetUIScale());
    UIScaleValueText->SetText(FText::AsPercent(HorrorGameSettings->GetUIScale()));
    UIScaleSlider->OnValueChanged.Clear();
    UIScaleSlider->OnValueChanged.AddDynamic(this, &UGameplayWidget::OnUIScaleChanged);

    // Tutorial Hints
    TutorialHintsCheckBox->SetIsChecked(HorrorGameSettings->GetTutorialHintsEnabled());
    TutorialHintsCheckBox->OnCheckStateChanged.Clear();
    TutorialHintsCheckBox->OnCheckStateChanged.AddDynamic(this, &UGameplayWidget::OnTutorialHintsChanged);

    // Camera Shake
    CameraShakeCheckBox->SetIsChecked(HorrorGameSettings->GetCameraShakeEnabled());
    CameraShakeCheckBox->OnCheckStateChanged.Clear();
    CameraShakeCheckBox->OnCheckStateChanged.AddDynamic(this, &UGameplayWidget::OnCameraShakeChanged);

    // Interaction Hold Time
    InteractionHoldTimeSlider->SetValue(HorrorGameSettings->GetInteractionHoldTime());
    InteractionHoldTimeText->SetText(FText::AsNumber(HorrorGameSettings->GetInteractionHoldTime(), &FNumberFormattingOptions::DefaultNoGrouping()));
    InteractionHoldTimeSlider->OnValueChanged.Clear();
    InteractionHoldTimeSlider->OnValueChanged.AddDynamic(this, &UGameplayWidget::OnInteractionHoldTimeChanged);
}

void UGameplayWidget::OnSubtitlesChanged(bool bIsChecked)
{
    HorrorGameSettings->SetSubtitlesEnabled(bIsChecked);
    HorrorGameSettings->SaveConfig();
}

void UGameplayWidget::OnAutoSaveChanged(bool bIsChecked)
{
    HorrorGameSettings->SetAutoSaveEnabled(bIsChecked);
    HorrorGameSettings->SaveConfig();
}

void UGameplayWidget::OnUIScaleChanged(float Value)
{
    HorrorGameSettings->SetUIScale(Value);
    HorrorGameSettings->SaveConfig();
    UIScaleValueText->SetText(FText::AsPercent(Value));

    // Apply UI scale immediately
    HorrorGameSettings->OnUIScaleChanged.Broadcast(Value);
}

void UGameplayWidget::OnTutorialHintsChanged(bool bIsChecked)
{
    HorrorGameSettings->SetTutorialHintsEnabled(bIsChecked);
    HorrorGameSettings->SaveConfig();
}

void UGameplayWidget::OnCameraShakeChanged(bool bIsChecked)
{
    HorrorGameSettings->SetCameraShakeEnabled(bIsChecked);
    HorrorGameSettings->SaveConfig();
}

void UGameplayWidget::OnInteractionHoldTimeChanged(float Value)
{
    HorrorGameSettings->SetInteractionHoldTime(Value);
    HorrorGameSettings->SaveConfig();
    InteractionHoldTimeText->SetText(FText::AsNumber(Value, &FNumberFormattingOptions::DefaultNoGrouping()));
}
