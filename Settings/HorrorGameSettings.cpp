#include "HorrorGameSettings.h"

UHorrorGameSettings::UHorrorGameSettings()
{
    CategoryName = TEXT("Horror Game");
}

UHorrorGameSettings* UHorrorGameSettings::Get()
{
    return GetMutableDefault<UHorrorGameSettings>();
}

FKey UHorrorGameSettings::GetKeyBinding(FName ActionName, FKey DefaultKey) const
{
    const FKey* FoundKey = KeyBindings.Find(ActionName);
    return FoundKey ? *FoundKey : DefaultKey;
}

void UHorrorGameSettings::SetKeyBinding(FName ActionName, FKey Key)
{
    KeyBindings.Add(ActionName, Key);
}

void UHorrorGameSettings::ResetKeyBindingsToDefault()
{
    KeyBindings.Empty();
    // Default bindings will be used when not found in the map
}

void UHorrorGameSettings::ResetAllToDefaults()
{
    Difficulty = 1;
    bSubtitlesEnabled = true;
    bAutoSaveEnabled = true;
    Language = TEXT("en");
    UIScale = 1.0f;
    bTutorialHintsEnabled = true;
    bCameraShakeEnabled = true;
    InteractionHoldTime = 1.0f;
    MouseSensitivity = 1.0f;
    bInvertMouseY = false;
    bInvertMouseX = false;
    ControllerSensitivity = 1.0f;
    bControllerVibrationEnabled = true;
    MasterVolume = 1.0f;
    MusicVolume = 1.0f;
    SFXVolume = 1.0f;
    VoiceVolume = 1.0f;
    AmbientVolume = 1.0f;
    bMuted = false;
    bColorBlindSupport = false;
    ColorBlindType = 0;
    FontSize = 1.0f;
    bHighContrastMode = false;
    bReduceMotion = false;

    ResetKeyBindingsToDefault();
    SaveConfig();
}