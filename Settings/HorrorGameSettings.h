#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "HorrorGameSettings.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChanged, const FString&, NewLanguage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIScaleChanged, float, NewScale);

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Horror Game Settings"))
class HORRORGAME_API UHorrorGameSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UHorrorGameSettings();

    static UHorrorGameSettings* Get();

    // Gameplay Settings
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    int32 Difficulty = 1; // 0=Easy, 1=Normal, 2=Hard, 3=Nightmare

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bSubtitlesEnabled = true;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAutoSaveEnabled = true;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    FString Language = TEXT("en");

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float UIScale = 1.0f;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bTutorialHintsEnabled = true;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bCameraShakeEnabled = true;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float InteractionHoldTime = 1.0f;

    // Controls Settings
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Controls", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float MouseSensitivity = 1.0f;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bInvertMouseY = false;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bInvertMouseX = false;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Controls", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float ControllerSensitivity = 1.0f;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bControllerVibrationEnabled = true;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Controls")
    TMap<FName, FKey> KeyBindings;

    // Audio Settings
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume = 1.0f;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume = 1.0f;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SFXVolume = 1.0f;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VoiceVolumevậy = 1.0f;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume = 1.0f;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bMuted = false;

    // Accessibility Settings
    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bColorBlindSupport = false;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    int32 ColorBlindType = 0; // 0=None, 1=Protanopia, 2=Deuteranopia, 3=Tritanopia

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float FontSize = 1.0f;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bHighContrastMode = false;

    UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
    bool bReduceMotion = false;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnLanguageChanged OnLanguageChanged;

    UPROPERTY(BlueprintAssignable)
    FOnUIScaleChanged OnUIScaleChanged;

    // Getters and Setters
    UFUNCTION(BlueprintCallable, Category = "Settings")
    int32 GetDifficulty() const { return Difficulty; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetDifficulty(int32 InDifficulty) { Difficulty = FMath::Clamp(InDifficulty, 0, 3); }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetSubtitlesEnabled() const { return bSubtitlesEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetSubtitlesEnabled(bool bEnabled) { bSubtitlesEnabled = bEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetAutoSaveEnabled() const { return bAutoSaveEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetAutoSaveEnabled(bool bEnabled) { bAutoSaveEnabled = bEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    FString GetLanguage() const { return Language; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetLanguage(const FString& InLanguage) { Language = InLanguage; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetUIScale() const { return UIScale; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetUIScale(float InScale) { UIScale = FMath::Clamp(InScale, 0.5f, 2.0f); }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetMouseSensitivity() const { return MouseSensitivity; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMouseSensitivity(float InSensitivity) { MouseSensitivity = FMath::Clamp(InSensitivity, 0.1f, 5.0f); }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetInvertMouseY() const { return bInvertMouseY; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetInvertMouseY(bool bInvert) { bInvertMouseY = bInvert; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool GetInvertMouseX() const { return bInvertMouseX; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetInvertMouseX(bool bInvert) { bInvertMouseX = bInvert; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    FKey GetKeyBinding(FName ActionName, FKey DefaultKey) const;

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetKeyBinding(FName ActionName, FKey Key);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ResetKeyBindingsToDefault();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ResetAllToDefaults();

    // Additional getters/setters...
    bool GetTutorialHintsEnabled() const { return bTutorialHintsEnabled; }
    void SetTutorialHintsEnabled(bool bEnabled) { bTutorialHintsEnabled = bEnabled; }

    bool GetCameraShakeEnabled() const { return bCameraShakeEnabled; }
    void SetCameraShakeEnabled(bool bEnabled) { bCameraShakeEnabled = bEnabled; }

    float GetInteractionHoldTime() const { return InteractionHoldTime; }
    void SetInteractionHoldTime(float Time) { InteractionHoldTime = FMath::Clamp(Time, 0.1f, 3.0f); }

    float GetControllerSensitivity() const { return ControllerSensitivity; }
    void SetControllerSensitivity(float InSensitivity) { ControllerSensitivity = FMath::Clamp(InSensitivity, 0.1f, 5.0f); }

    bool GetControllerVibrationEnabled() const { return bControllerVibrationEnabled; }
    void SetControllerVibrationEnabled(bool bEnabled) { bControllerVibrationEnabled = bEnabled; }
};