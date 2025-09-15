#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "GameFramework/InputSettings.h"
#include "UObject/UnrealType.h"

UHorrorGameSettings::UHorrorGameSettings()
{
    // Gameplay defaults
    Difficulty = 1;
    Language = TEXT("en");
    bSubtitlesEnabled = true;
    bAutoSaveEnabled = true;
    bTutorialHintsEnabled = true;
    bCameraShakeEnabled = true;
    UIScale = 1.0f;
    InteractionHoldTime = 0.5f;

    // Audio defaults
    MasterVolume = 0.8f;
    MusicVolume = 0.7f;
    SFXVolume = 0.8f;
    VoiceVolume = 0.8f;
    AmbientVolume = 0.7f;
    bMasterMuted = false;
    bMusicMuted = false;
    bSFXMuted = false;
    bVoiceMuted = false;
    bAmbientMuted = false;
    AudioQuality = 2;
    bSpatialAudioEnabled = true;
    bAudioCompressionEnabled = true;

    // Controls defaults
    MouseSensitivity = 1.0f;
    ControllerSensitivity = 1.0f;
    bInvertMouseY = false;
    bInvertMouseX = false;
    bControllerVibrationEnabled = true;
    bMouseSmoothingEnabled = true;
    ControllerDeadZone = 0.2f;
    bMouseAccelerationEnabled = false;
    bRawInputEnabled = true;

    // Graphics
    FieldOfView = 90.0f;

    // Accessibility defaults
    bColorBlindSupport = false;
    ColorBlindType = 0;
    FontSize = 1.0f;
    bHighContrastMode = false;
    bReduceMotion = false;
    bFlashingLightsReduction = false;
    ScreenReaderSpeed = 1.0f;
    bClosedCaptionsEnabled = false;
    CaptionSize = 1.0f;
    bScreenFlashReduction = false;
    bAudioCuesEnabled = true;
    UIContrast = 1.0f;

    // Voice Settings Defaults
    VoiceType = 0; // Push to Talk
    PushToTalkKey = EKeys::V;
    VoiceThreshold = 0.3f;
    VoiceGain = 1.0f;
    bVoiceNoiseGate = true;
    NoiseGateThreshold = 0.1f;
    bVoiceEcho = false;
    VoiceEchoStrength = 0.3f;
    bProximityVoice = true;
    ProximityVoiceRange = 20.0f;
    bVoiceDistortion = false;
    VoiceDistortionIntensity = 0.5f;
    bWhisperMode = false;
    PreferredInputDevice = TEXT("Default");
    bVoiceActivityDetection = true;
    bSpeechToText = false;
    SpeechLanguage = TEXT("en-US");

    // Other
    bIsSettingsChanged = false;

    // Default key bindings
    ResetKeyBindingsToDefault();
    InitializeDefaultGamepadKeyBindings();
}

UHorrorGameSettings* UHorrorGameSettings::Get()
{
    return GetMutableDefault<UHorrorGameSettings>();
}

// Gameplay Settings
void UHorrorGameSettings::SetDifficulty(int32 InDifficulty)
{
    Difficulty = FMath::Clamp(InDifficulty, 0, 3);
    SaveConfig();
}

void UHorrorGameSettings::SetSubtitlesEnabled(bool bEnabled)
{
    bSubtitlesEnabled = bEnabled;
    SaveConfig();
}

void UHorrorGameSettings::SetAutoSaveEnabled(bool bEnabled)
{
    bAutoSaveEnabled = bEnabled;
    SaveConfig();
}

void UHorrorGameSettings::SetLanguage(const FString& InLanguage)
{
    if (Language.Equals(InLanguage))
    {
        return;
    }

    Language = InLanguage;
    SaveConfig();
    OnLanguageChanged.Broadcast(Language);
}

void UHorrorGameSettings::SetUIScale(float InScale)
{
    float NewScale = FMath::Clamp(InScale, 0.5f, 2.0f);
    if (FMath::IsNearlyEqual(UIScale, NewScale))
    {
        return;
    }

    UIScale = NewScale;
    SaveConfig();
    OnUIScaleChanged.Broadcast(UIScale);
}

void UHorrorGameSettings::SetTutorialHintsEnabled(bool bEnabled)
{
    bTutorialHintsEnabled = bEnabled;
    SaveConfig();
}

void UHorrorGameSettings::SetCameraShakeEnabled(bool bEnabled)
{
    bCameraShakeEnabled = bEnabled;
    SaveConfig();
}

void UHorrorGameSettings::SetInteractionHoldTime(float Time)
{
    InteractionHoldTime = FMath::Clamp(Time, 0.1f, 3.0f);
    SaveConfig();
}

// Audio Settings
void UHorrorGameSettings::SetMasterVolume(float InVolume)
{
    MasterVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
    SaveConfig();
}

void UHorrorGameSettings::SetMusicVolume(float InVolume)
{
    MusicVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
    SaveConfig();
}

void UHorrorGameSettings::SetSFXVolume(float InVolume)
{
    SFXVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
    SaveConfig();
}

void UHorrorGameSettings::SetAmbientVolume(float InVolume)
{
    AmbientVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
    SaveConfig();
}

// Controls Settings
void UHorrorGameSettings::SetMouseSensitivity(float InSensitivity)
{
    MouseSensitivity = FMath::Clamp(InSensitivity, 0.1f, 5.0f);
    SaveConfig();
}

void UHorrorGameSettings::SetInvertMouseY(bool bInvert)
{
    bInvertMouseY = bInvert;
    SaveConfig();
}

void UHorrorGameSettings::SetInvertMouseX(bool bInvert)
{
    bInvertMouseX = bInvert;
    SaveConfig();
}

void UHorrorGameSettings::SetControllerSensitivity(float InSensitivity)
{
    ControllerSensitivity = FMath::Clamp(InSensitivity, 0.1f, 5.0f);
    SaveConfig();
}

void UHorrorGameSettings::SetControllerVibrationEnabled(bool bEnabled)
{
    bControllerVibrationEnabled = bEnabled;
    SaveConfig();
}

void UHorrorGameSettings::SetMouseSmoothingEnabled(bool bEnabled)
{
    bMouseSmoothingEnabled = bEnabled;
    SaveConfig();
}

void UHorrorGameSettings::SetMouseAccelerationEnabled(bool bEnabled)
{
    bMouseAccelerationEnabled = bEnabled;
    SaveConfig();
}

void UHorrorGameSettings::SetControllerDeadZone(float InDeadZone)
{
    ControllerDeadZone = FMath::Clamp(InDeadZone, 0.0f, 1.0f);
    SaveConfig();
}

// Graphics Settings
void UHorrorGameSettings::SetFieldOfView(float InFOV)
{
    float NewFOV = FMath::Clamp(InFOV, 60.0f, 120.0f);
    if (FMath::IsNearlyEqual(FieldOfView, NewFOV))
    {
        return;
    }
    FieldOfView = NewFOV;
    SaveConfig();
    OnFOVChanged.Broadcast(FieldOfView);
}

// Accessibility Settings
void UHorrorGameSettings::SetColorBlindSupport(bool bEnabled)
{
    if (bColorBlindSupport != bEnabled)
    {
        bColorBlindSupport = bEnabled;
        SaveConfig();
    }
}

void UHorrorGameSettings::SetColorBlindType(int32 InType)
{
    int32 ClampedType = FMath::Clamp(InType, 0, 3);
    if (ColorBlindType != ClampedType)
    {
        ColorBlindType = ClampedType;
        SaveConfig();
    }
}

void UHorrorGameSettings::SetFontSize(float InSize)
{
    float ClampedSize = FMath::Clamp(InSize, 0.5f, 2.0f);
    if (!FMath::IsNearlyEqual(FontSize, ClampedSize))
    {
        FontSize = ClampedSize;
        SaveConfig();
        OnFontSizeChanged.Broadcast(FontSize);
    }
}

void UHorrorGameSettings::SetHighContrastMode(bool bEnabled)
{
    if (bHighContrastMode != bEnabled)
    {
        bHighContrastMode = bEnabled;
        SaveConfig();
        OnHighContrastChanged.Broadcast(bHighContrastMode);
    }
}

void UHorrorGameSettings::SetReduceMotion(bool bEnabled)
{
    if (bReduceMotion != bEnabled)
    {
        bReduceMotion = bEnabled;
        SaveConfig();
        OnMotionReductionChanged.Broadcast(bReduceMotion);
    }
}

void UHorrorGameSettings::SetFlashingLightsReduction(bool bEnabled)
{
    if (bFlashingLightsReduction != bEnabled)
    {
        bFlashingLightsReduction = bEnabled;
        SaveConfig();
        OnFlashingLightsReductionChanged.Broadcast(bFlashingLightsReduction);
    }
}

void UHorrorGameSettings::SetClosedCaptionsEnabled(bool bEnabled)
{
    if (bClosedCaptionsEnabled != bEnabled)
    {
        bClosedCaptionsEnabled = bEnabled;
        SaveConfig();
        OnClosedCaptionsChanged.Broadcast(bClosedCaptionsEnabled);
    }
}

void UHorrorGameSettings::SetCaptionSize(float InSize)
{
    float ClampedSize = FMath::Clamp(InSize, 0.5f, 2.0f);
    if (!FMath::IsNearlyEqual(CaptionSize, ClampedSize))
    {
        CaptionSize = ClampedSize;
        SaveConfig();
        OnCaptionSizeChanged.Broadcast(CaptionSize);
    }
}

// Voice Settings
void UHorrorGameSettings::SetVoiceType(int32 InVoiceType)
{
    if (VoiceType != InVoiceType)
    {
        VoiceType = FMath::Clamp(InVoiceType, 0, 2);
        SaveConfig();
        OnVoiceTypeChanged.Broadcast(VoiceType);
        OnSettingsSimpleChanged.Broadcast();
    }
}

void UHorrorGameSettings::SetPushToTalkKey(FKey InKey)
{
    if (PushToTalkKey != InKey)
    {
        PushToTalkKey = InKey;
        SaveConfig();
        OnPushToTalkKeyChanged.Broadcast(PushToTalkKey);
        OnSettingsChangedWithParam.Broadcast(this);
    }
}

void UHorrorGameSettings::SetVoiceThreshold(float InThreshold)
{
    float ClampedThreshold = FMath::Clamp(InThreshold, 0.0f, 1.0f);
    if (!FMath::IsNearlyEqual(VoiceThreshold, ClampedThreshold))
    {
        VoiceThreshold = ClampedThreshold;
        SaveConfig();
        OnVoiceThresholdChanged.Broadcast(VoiceThreshold);
        OnSettingsSimpleChanged.Broadcast();
    }
}

void UHorrorGameSettings::SetVoiceMuted(bool bMuted)
{
    if (bVoiceMuted != bMuted)
    {
        bVoiceMuted = bMuted;
        SaveConfig();
        OnVoiceChanged.Broadcast(!bMuted);
        OnSettingsSimpleChanged.Broadcast();
    }
}

void UHorrorGameSettings::SetVoiceVolume(float InVolume)
{
    float ClampedVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
    if (!FMath::IsNearlyEqual(VoiceVolume, ClampedVolume))
    {
        VoiceVolume = ClampedVolume;
        SaveConfig();
        OnVoiceChanged.Broadcast(true);
        OnSettingsSimpleChanged.Broadcast();
    }
}

void UHorrorGameSettings::SetProximityVoice(bool bEnabled)
{
    if (bProximityVoice != bEnabled)
    {
        bProximityVoice = bEnabled;
        SaveConfig();
        OnProximityVoiceChanged.Broadcast(bEnabled);
        OnSettingsSimpleChanged.Broadcast();
    }
}

void UHorrorGameSettings::SetWhisperMode(bool bEnabled)
{
    if (bWhisperMode != bEnabled)
    {
        bWhisperMode = bEnabled;
        SaveConfig();
        OnVoiceChanged.Broadcast(true);
        OnSettingsSimpleChanged.Broadcast();
    }
}

void UHorrorGameSettings::SetVoiceDistortion(bool bEnabled)
{
    if (bVoiceDistortion != bEnabled)
    {
        bVoiceDistortion = bEnabled;
        SaveConfig();
        OnVoiceChanged.Broadcast(true);
        OnSettingsSimpleChanged.Broadcast();
    }
}

// Key Bindings
FKey UHorrorGameSettings::GetKeyBinding(FName ActionName, FKey DefaultKey) const
{
    const FKey* Found = KeyBindings.Find(ActionName);
    return Found ? *Found : DefaultKey;
}

void UHorrorGameSettings::SetKeyBinding(FName ActionName, FKey Key)
{
    KeyBindings.Add(ActionName, Key);
    SaveConfig();

    // Try to update project input settings to keep editor and runtime in sync
    if (UInputSettings* InputSettings = const_cast<UInputSettings*>(UInputSettings::GetInputSettings()))
    {
        // Update any action mapping with same name; if not exist, add one mapping
        bool bUpdated = false;
        TArray<FInputActionKeyMapping> ActionMappings = InputSettings->GetActionMappings();
        for (FInputActionKeyMapping& Mapping : ActionMappings)
        {
            if (Mapping.ActionName == ActionName)
            {
                Mapping.Key = Key;
                bUpdated = true;
            }
        }

        if (!bUpdated)
        {
            InputSettings->AddActionMapping(FInputActionKeyMapping(ActionName, Key));
        }

        InputSettings->SaveKeyMappings();
        InputSettings->ForceRebuildKeymaps();
    }
}

void UHorrorGameSettings::SetRawInputEnabled(bool bEnabled)
{
    if (bRawInputEnabled != bEnabled)
    {
        bRawInputEnabled = bEnabled;
        SaveConfig();

        UE_LOG(LogTemp, Log, TEXT("Raw input %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
    }
}

FKey UHorrorGameSettings::GetGamepadKeyBinding(FName ActionName, FKey DefaultKey) const
{
    if (const FKey* FoundKey = GamepadKeyBindings.Find(ActionName))
    {
        return *FoundKey;
    }
    return DefaultKey;
}

void UHorrorGameSettings::SetGamepadKeyBinding(FName ActionName, FKey Key)
{
    if (!Key.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to set invalid gamepad key for action %s"), *ActionName.ToString());
        return;
    }

    // Update or add the key binding
    GamepadKeyBindings.Add(ActionName, Key);
    SaveConfig();

    UE_LOG(LogTemp, Log, TEXT("Gamepad key binding updated: %s -> %s"), *ActionName.ToString(), *Key.ToString());
}

void UHorrorGameSettings::ResetGamepadKeyBindingsToDefault()
{
    GamepadKeyBindings.Empty();
    InitializeDefaultGamepadKeyBindings();
    SaveConfig();

    UE_LOG(LogTemp, Log, TEXT("Gamepad key bindings reset to defaults"));
}

void UHorrorGameSettings::InitializeDefaultGamepadKeyBindings()
{
    // Set up default gamepad key bindings for horror game
    GamepadKeyBindings.Add(TEXT("MoveForward"), EKeys::Gamepad_LeftStick_Up);
    GamepadKeyBindings.Add(TEXT("MoveBackward"), EKeys::Gamepad_LeftStick_Down);
    GamepadKeyBindings.Add(TEXT("MoveLeft"), EKeys::Gamepad_LeftStick_Left);
    GamepadKeyBindings.Add(TEXT("MoveRight"), EKeys::Gamepad_LeftStick_Right);

    GamepadKeyBindings.Add(TEXT("LookUp"), EKeys::Gamepad_RightStick_Up);
    GamepadKeyBindings.Add(TEXT("LookRight"), EKeys::Gamepad_RightStick_Right);

    GamepadKeyBindings.Add(TEXT("Jump"), EKeys::Gamepad_FaceButton_Bottom);
    GamepadKeyBindings.Add(TEXT("Crouch"), EKeys::Gamepad_RightThumbstick);
    GamepadKeyBindings.Add(TEXT("Run"), EKeys::Gamepad_LeftShoulder);
    GamepadKeyBindings.Add(TEXT("Walk"), EKeys::Gamepad_LeftTrigger);

    GamepadKeyBindings.Add(TEXT("Interact"), EKeys::Gamepad_FaceButton_Right);
    GamepadKeyBindings.Add(TEXT("UseItem"), EKeys::Gamepad_RightTrigger);
    GamepadKeyBindings.Add(TEXT("DropItem"), EKeys::Gamepad_FaceButton_Top);

    // Horror-specific gamepad bindings
    GamepadKeyBindings.Add(TEXT("Flashlight"), EKeys::Gamepad_FaceButton_Left);
    GamepadKeyBindings.Add(TEXT("Hide"), EKeys::Gamepad_RightShoulder);
    GamepadKeyBindings.Add(TEXT("HoldBreath"), EKeys::Gamepad_LeftTrigger);
    GamepadKeyBindings.Add(TEXT("LeanLeft"), EKeys::Gamepad_DPad_Left);
    GamepadKeyBindings.Add(TEXT("LeanRight"), EKeys::Gamepad_DPad_Right);

    GamepadKeyBindings.Add(TEXT("Inventory"), EKeys::Gamepad_Special_Right);
    GamepadKeyBindings.Add(TEXT("PauseMenu"), EKeys::Gamepad_Special_Left);
    GamepadKeyBindings.Add(TEXT("Journal"), EKeys::Gamepad_DPad_Up);
    GamepadKeyBindings.Add(TEXT("Map"), EKeys::Gamepad_DPad_Down);
}

void UHorrorGameSettings::ResetKeyBindingsToDefault()
{
    KeyBindings.Empty();
    // Common default action names — change these to match your project's InputSettings if needed
    KeyBindings.Add("MoveForward", EKeys::W);
    KeyBindings.Add("MoveBackward", EKeys::S);
    KeyBindings.Add("MoveRight", EKeys::D);
    KeyBindings.Add("MoveLeft", EKeys::A);
    KeyBindings.Add("Jump", EKeys::SpaceBar);
    KeyBindings.Add("Interact", EKeys::E);
    KeyBindings.Add("Sprint", EKeys::LeftShift);
    KeyBindings.Add("Crouch", EKeys::C);

    SaveConfig();

    // Also attempt to write those back to project input settings so editor uses them too
    if (UInputSettings* InputSettings = const_cast<UInputSettings*>(UInputSettings::GetInputSettings()))
    {
        // Remove action mappings that match keys we set, then add ours
        for (const TPair<FName, FKey>& Pair : KeyBindings)
        {
            // Remove existing mappings with same action to avoid duplicates
            InputSettings->RemoveActionMapping(FInputActionKeyMapping(Pair.Key, FKey()));
            InputSettings->AddActionMapping(FInputActionKeyMapping(Pair.Key, Pair.Value));
        }
        InputSettings->SaveKeyMappings();
        InputSettings->ForceRebuildKeymaps();
    }
}

void UHorrorGameSettings::ResetAllToDefaults()
{
    const UHorrorGameSettings* Default = GetDefault<UHorrorGameSettings>();
    if (!Default)
    {
        return;
    }

    // Gameplay Settings
    Difficulty = Default->Difficulty;
    Language = Default->Language;
    bSubtitlesEnabled = Default->bSubtitlesEnabled;
    bAutoSaveEnabled = Default->bAutoSaveEnabled;
    bTutorialHintsEnabled = Default->bTutorialHintsEnabled;
    bCameraShakeEnabled = Default->bCameraShakeEnabled;
    UIScale = Default->UIScale;
    InteractionHoldTime = Default->InteractionHoldTime;

    // Audio Settings
    MasterVolume = Default->MasterVolume;
    MusicVolume = Default->MusicVolume;
    SFXVolume = Default->SFXVolume;
    VoiceVolume = Default->VoiceVolume;
    AmbientVolume = Default->AmbientVolume;
    bMasterMuted = Default->bMasterMuted;
    bMusicMuted = Default->bMusicMuted;
    bSFXMuted = Default->bSFXMuted;
    bVoiceMuted = Default->bVoiceMuted;
    bAmbientMuted = Default->bAmbientMuted;
    AudioQuality = Default->AudioQuality;
    bSpatialAudioEnabled = Default->bSpatialAudioEnabled;
    bAudioCompressionEnabled = Default->bAudioCompressionEnabled;

    // Controls Settings
    MouseSensitivity = Default->MouseSensitivity;
    ControllerSensitivity = Default->ControllerSensitivity;
    bInvertMouseY = Default->bInvertMouseY;
    bInvertMouseX = Default->bInvertMouseX;
    bControllerVibrationEnabled = Default->bControllerVibrationEnabled;
    bMouseSmoothingEnabled = Default->bMouseSmoothingEnabled;
    ControllerDeadZone = Default->ControllerDeadZone;
    bMouseAccelerationEnabled = Default->bMouseAccelerationEnabled;
    KeyBindings = Default->KeyBindings;

    // Graphics Settings
    FieldOfView = Default->FieldOfView;

    // Accessibility Settings
    bColorBlindSupport = Default->bColorBlindSupport;
    ColorBlindType = Default->ColorBlindType;
    FontSize = Default->FontSize;
    bHighContrastMode = Default->bHighContrastMode;
    bReduceMotion = Default->bReduceMotion;
    bFlashingLightsReduction = Default->bFlashingLightsReduction;
    ScreenReaderSpeed = Default->ScreenReaderSpeed;
    bClosedCaptionsEnabled = Default->bClosedCaptionsEnabled;
    CaptionSize = Default->CaptionSize;
    bScreenFlashReduction = Default->bScreenFlashReduction;
    bAudioCuesEnabled = Default->bAudioCuesEnabled;
    UIContrast = Default->UIContrast;

    // Voice Settings
    VoiceType = Default->VoiceType;
    PushToTalkKey = Default->PushToTalkKey;
    VoiceThreshold = Default->VoiceThreshold;
    VoiceGain = Default->VoiceGain;
    bVoiceNoiseGate = Default->bVoiceNoiseGate;
    NoiseGateThreshold = Default->NoiseGateThreshold;
    bVoiceEcho = Default->bVoiceEcho;
    VoiceEchoStrength = Default->VoiceEchoStrength;
    bProximityVoice = Default->bProximityVoice;
    ProximityVoiceRange = Default->ProximityVoiceRange;
    bVoiceDistortion = Default->bVoiceDistortion;
    VoiceDistortionIntensity = Default->VoiceDistortionIntensity;
    bWhisperMode = Default->bWhisperMode;
    PreferredInputDevice = Default->PreferredInputDevice;
    bVoiceActivityDetection = Default->bVoiceActivityDetection;
    bSpeechToText = Default->bSpeechToText;
    SpeechLanguage = Default->SpeechLanguage;

    // Other Settings
    bIsSettingsChanged = Default->bIsSettingsChanged;

    SaveConfig();

    // Broadcast changes so UI can update
    OnLanguageChanged.Broadcast(Language);
    OnUIScaleChanged.Broadcast(UIScale);
    OnFOVChanged.Broadcast(FieldOfView);
    OnFontSizeChanged.Broadcast(FontSize);
    OnHighContrastChanged.Broadcast(bHighContrastMode);
    OnMotionReductionChanged.Broadcast(bReduceMotion);
    OnFlashingLightsReductionChanged.Broadcast(bFlashingLightsReduction);
    OnClosedCaptionsChanged.Broadcast(bClosedCaptionsEnabled);
    OnCaptionSizeChanged.Broadcast(CaptionSize);
    OnScreenFlashReductionChanged.Broadcast(bScreenFlashReduction);
    OnAudioCuesChanged.Broadcast(bAudioCuesEnabled);
    OnUIContrastChanged.Broadcast(UIContrast);
    OnScreenReaderSpeedChanged.Broadcast(ScreenReaderSpeed);
    OnVoiceChanged.Broadcast(!bVoiceMuted);
    OnVoiceTypeChanged.Broadcast(VoiceType);
    OnVoiceThresholdChanged.Broadcast(VoiceThreshold);
    OnPushToTalkKeyChanged.Broadcast(PushToTalkKey);
    OnProximityVoiceChanged.Broadcast(bProximityVoice);
    OnSettingsSimpleChanged.Broadcast();
    OnSettingsChangedWithParam.Broadcast(this);
}