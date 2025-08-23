#include "SettingsManager.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void USettingsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Bind to settings change events
    if (UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get())
    {
        GameSettings->OnLanguageChanged.AddDynamic(this, &USettingsManager::OnLanguageChanged);
        GameSettings->OnUIScaleChanged.AddDynamic(this, &USettingsManager::OnUIScaleChanged);
    }

    LoadAllSettings();
}

void USettingsManager::Deinitialize()
{
    if (bHasUnsavedChanges)
    {
        SaveAllSettings();
    }

    Super::Deinitialize();
}

void USettingsManager::ApplyGraphicsSettings()
{
    if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
    {
        GameUserSettings->ApplySettings(false);
    }

    OnSettingsChanged.Broadcast(FName("Graphics"));
}

void USettingsManager::ApplyAudioSettings()
{
    UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get();
    if (!GameSettings) return;

    // Apply master volume
    // TODO: Implement audio system integration

    OnSettingsChanged.Broadcast(FName("Audio"));
}

void USettingsManager::ApplyGameplaySettings()
{
    UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get();
    if (!GameSettings) return;

    // Apply difficulty settings to game mode
    // TODO: Notify game mode of difficulty change

    OnSettingsChanged.Broadcast(FName("Gameplay"));
}

void USettingsManager::ApplyControlSettings()
{
    UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get();
    if (!GameSettings) return;

    // Apply input settings
    // TODO: Update input component sensitivity settings

    OnSettingsChanged.Broadcast(FName("Controls"));
}

void USettingsManager::ApplyAccessibilitySettings()
{
    UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get();
    if (!GameSettings) return;

    // Apply accessibility settings
    // Color blind filter
    if (GameSettings->bColorBlindSupport)
    {
        // TODO: Apply color blind filter based on type
    }

    // Font size scaling
    if (GameSettings->FontSize != 1.0f)
    {
        // TODO: Update all UI text scaling
    }

    // High contrast mode
    if (GameSettings->bHighContrastMode)
    {
        // TODO: Switch to high contrast UI theme
    }

    OnSettingsChanged.Broadcast(FName("Accessibility"));
}

void USettingsManager::SaveAllSettings()
{
    if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
    {
        GameUserSettings->SaveSettings();
    }

    if (UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get())
    {
        GameSettings->SaveConfig();
    }

    bHasUnsavedChanges = false;
}

void USettingsManager::LoadAllSettings()
{
    if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
    {
        GameUserSettings->LoadSettings();
    }

    // Apply all settings on load
    ApplyGraphicsSettings();
    ApplyAudioSettings();
    ApplyGameplaySettings();
    ApplyControlSettings();
    ApplyAccessibilitySettings();

    bHasUnsavedChanges = false;
}

void USettingsManager::OnLanguageChanged(const FString& NewLanguage)
{
    // Reload localization
    // TODO: Implement localization reloading
    bHasUnsavedChanges = true;
}

void USettingsManager::OnUIScaleChanged(float NewScale)
{
    // Apply UI scaling to all widgets
    // TODO: Implement UI scaling system
    bHasUnsavedChanges = true;
}