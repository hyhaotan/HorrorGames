#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "HorrorGameSettings.generated.h"

// Delegates for settings changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChanged, const FString&, NewLanguage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIScaleChanged, float, NewScale);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFOVChanged, float, NewFOV);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFontSizeChanged, float, NewFontSizeMultiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighContrastChanged, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMotionReductionChanged, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashingLightsReductionChanged, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClosedCaptionsChanged, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCaptionSizeChanged, float, NewSize);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenFlashReductionChanged, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioCuesChanged, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIContrastChanged, float, NewContrast);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenReaderSpeedChanged, float, NewSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceTypeChanged, int32, NewVoiceType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceThresholdChanged, float, NewThreshold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPushToTalkKeyChanged, FKey, NewKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProximityVoiceChanged, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceChanged, bool, bEnable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsChangedWithParam, UHorrorGameSettings*, Settings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsSimpleChanged);

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Horror Game Settings"))
class HORRORGAME_API UHorrorGameSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UHorrorGameSettings();

    // Returns a mutable default for runtime editing
    static UHorrorGameSettings* Get();

    // Gameplay Settings
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0", ClampMax = "3"))
    int32 Difficulty;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    FString Language;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bSubtitlesEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAutoSaveEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bTutorialHintsEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bCameraShakeEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float UIScale;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float InteractionHoldTime;

    // Audio Settings
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SFXVolume;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VoiceVolume;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bMasterMuted;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bMusicMuted;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bSFXMuted;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bVoiceMuted;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bAmbientMuted;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0", ClampMax = "3"))
    int32 AudioQuality;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bSpatialAudioEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bAudioCompressionEnabled;

    // Controls Settings
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Controls", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float MouseSensitivity;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Controls", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float ControllerSensitivity;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bInvertMouseY;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bInvertMouseX;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bControllerVibrationEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bMouseSmoothingEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Controls", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ControllerDeadZone;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bMouseAccelerationEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    TMap<FName, FKey> KeyBindings;

    UPROPERTY(Config, BlueprintReadOnly, Category = "Controls")
    bool bRawInputEnabled;

    UPROPERTY(Config, BlueprintReadOnly, Category = "Controls")
    TMap<FName, FKey> GamepadKeyBindings;

    // Graphics Settings
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "60.0", ClampMax = "120.0"))
    float FieldOfView;

    // Accessibility Settings
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bColorBlindSupport;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (ClampMin = "0", ClampMax = "3"))
    int32 ColorBlindType;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float FontSize;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bHighContrastMode;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bReduceMotion;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bFlashingLightsReduction;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float ScreenReaderSpeed;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bClosedCaptionsEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float CaptionSize;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bScreenFlashReduction;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bAudioCuesEnabled;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float UIContrast;

    // Voice Settings
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = "0", ClampMax = "2"))
    int32 VoiceType;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FKey PushToTalkKey;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VoiceThreshold;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float VoiceGain;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bVoiceNoiseGate;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoiseGateThreshold;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bVoiceEcho;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VoiceEchoStrength;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bProximityVoice;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = "5.0", ClampMax = "50.0"))
    float ProximityVoiceRange;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bVoiceDistortion;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VoiceDistortionIntensity;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bWhisperMode;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString PreferredInputDevice;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bVoiceActivityDetection;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bSpeechToText;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString SpeechLanguage;

    // Other Settings
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bIsSettingsChanged;

    // Events (BlueprintAssignable so UI/blueprints can bind)
    UPROPERTY(BlueprintAssignable)
    FOnLanguageChanged OnLanguageChanged;

    UPROPERTY(BlueprintAssignable)
    FOnUIScaleChanged OnUIScaleChanged;

    UPROPERTY(BlueprintAssignable)
    FOnFOVChanged OnFOVChanged;

    UPROPERTY(BlueprintAssignable)
    FOnFontSizeChanged OnFontSizeChanged;

    UPROPERTY(BlueprintAssignable)
    FOnHighContrastChanged OnHighContrastChanged;

    UPROPERTY(BlueprintAssignable)
    FOnMotionReductionChanged OnMotionReductionChanged;

    UPROPERTY(BlueprintAssignable)
    FOnFlashingLightsReductionChanged OnFlashingLightsReductionChanged;

    UPROPERTY(BlueprintAssignable)
    FOnClosedCaptionsChanged OnClosedCaptionsChanged;

    UPROPERTY(BlueprintAssignable)
    FOnCaptionSizeChanged OnCaptionSizeChanged;

    UPROPERTY(BlueprintAssignable)
    FOnScreenFlashReductionChanged OnScreenFlashReductionChanged;

    UPROPERTY(BlueprintAssignable)
    FOnAudioCuesChanged OnAudioCuesChanged;

    UPROPERTY(BlueprintAssignable)
    FOnUIContrastChanged OnUIContrastChanged;

    UPROPERTY(BlueprintAssignable)
    FOnScreenReaderSpeedChanged OnScreenReaderSpeedChanged;

    UPROPERTY(BlueprintAssignable)
    FOnVoiceTypeChanged OnVoiceTypeChanged;

    UPROPERTY(BlueprintAssignable)
    FOnVoiceThresholdChanged OnVoiceThresholdChanged;

    UPROPERTY(BlueprintAssignable)
    FOnPushToTalkKeyChanged OnPushToTalkKeyChanged;

    UPROPERTY(BlueprintAssignable)
    FOnProximityVoiceChanged OnProximityVoiceChanged;

    UPROPERTY(BlueprintAssignable)
    FOnVoiceChanged OnVoiceChanged;

    UPROPERTY(BlueprintAssignable)
    FOnSettingsSimpleChanged OnSettingsSimpleChanged;

    UPROPERTY(BlueprintAssignable)
    FOnSettingsChangedWithParam OnSettingsChangedWithParam;

public:
    // Gameplay - Getters & Setters
    UFUNCTION(BlueprintCallable, Category = "Settings")
    int32 GetDifficulty() const { return Difficulty; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetDifficulty(int32 InDifficulty);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetSubtitlesEnabled() const { return bSubtitlesEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetSubtitlesEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetAutoSaveEnabled() const { return bAutoSaveEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetAutoSaveEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    FString GetLanguage() const { return Language; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetLanguage(const FString& InLanguage);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetUIScale() const { return UIScale; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetUIScale(float InScale);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetTutorialHintsEnabled() const { return bTutorialHintsEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetTutorialHintsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetCameraShakeEnabled() const { return bCameraShakeEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetCameraShakeEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetInteractionHoldTime() const { return InteractionHoldTime; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetInteractionHoldTime(float Time);

    // Audio - Getters & Setters
    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetMasterVolume() const { return MasterVolume; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMasterVolume(float InVolume);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetMusicVolume() const { return MusicVolume; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMusicVolume(float InVolume);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetSFXVolume() const { return SFXVolume; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetSFXVolume(float InVolume);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetAmbientVolume() const { return AmbientVolume; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetAmbientVolume(float InVolume);

    // Controls - Getters & Setters
    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetMouseSensitivity() const { return MouseSensitivity; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMouseSensitivity(float InSensitivity);

    UFUNCTION(BlueprintCallable, Category = "Controls")
    bool GetRawInputEnabled() const { return bRawInputEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Controls")
    void SetRawInputEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Controls")
    FKey GetGamepadKeyBinding(FName ActionName, FKey DefaultKey) const;

    UFUNCTION(BlueprintCallable, Category = "Controls")
    void SetGamepadKeyBinding(FName ActionName, FKey Key);

    UFUNCTION(BlueprintCallable, Category = "Controls")
    void ResetGamepadKeyBindingsToDefault();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetInvertMouseY() const { return bInvertMouseY; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetInvertMouseY(bool bInvert);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetInvertMouseX() const { return bInvertMouseX; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetInvertMouseX(bool bInvert);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetControllerSensitivity() const { return ControllerSensitivity; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetControllerSensitivity(float InSensitivity);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetControllerVibrationEnabled() const { return bControllerVibrationEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetControllerVibrationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetMouseSmoothingEnabled() const { return bMouseSmoothingEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMouseSmoothingEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetMouseAccelerationEnabled() const { return bMouseSmoothingEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMouseAccelerationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetControllerDeadZone() const { return ControllerDeadZone; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetControllerDeadZone(float InDeadZone);

    // Graphics - Getters & Setters
    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetFieldOfView() const { return FieldOfView; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetFieldOfView(float InFOV);

    // Accessibility - Getters & Setters
    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetColorBlindSupport() const { return bColorBlindSupport; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetColorBlindSupport(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    int32 GetColorBlindType() const { return ColorBlindType; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetColorBlindType(int32 InType);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetFontSize() const { return FontSize; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetFontSize(float InSize);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetHighContrastMode() const { return bHighContrastMode; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetHighContrastMode(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetReduceMotion() const { return bReduceMotion; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetReduceMotion(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetFlashingLightsReduction() const { return bFlashingLightsReduction; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetFlashingLightsReduction(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetClosedCaptionsEnabled() const { return bClosedCaptionsEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetClosedCaptionsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetCaptionSize() const { return CaptionSize; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetCaptionSize(float InSize);

    // Voice Settings - Getters & Setters
    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    int32 GetVoiceType() const { return VoiceType; }

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void SetVoiceType(int32 InVoiceType);

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    FKey GetPushToTalkKey() const { return PushToTalkKey; }

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void SetPushToTalkKey(FKey InKey);

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    float GetVoiceThreshold() const { return VoiceThreshold; }

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void SetVoiceThreshold(float InThreshold);

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    bool GetVoiceMuted() const { return bVoiceMuted; }

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void SetVoiceMuted(bool bMuted);

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    float GetVoiceVolume() const { return VoiceVolume; }

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void SetVoiceVolume(float InVolume);

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    bool GetProximityVoice() const { return bProximityVoice; }

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void SetProximityVoice(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    bool GetWhisperMode() const { return bWhisperMode; }

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void SetWhisperMode(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    bool GetVoiceDistortion() const { return bVoiceDistortion; }

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void SetVoiceDistortion(bool bEnabled);

    // Key Bindings - Helper functions
    UFUNCTION(BlueprintCallable, Category = "Settings")
    FKey GetKeyBinding(FName ActionName, FKey DefaultKey) const;

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetKeyBinding(FName ActionName, FKey Key);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ResetKeyBindingsToDefault();

    // Reset - Helper
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ResetAllToDefaults();

private:
    void InitializeDefaultGamepadKeyBindings();
};