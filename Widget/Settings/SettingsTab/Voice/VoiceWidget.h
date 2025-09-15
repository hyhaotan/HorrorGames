#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "VoiceWidget.generated.h"

UENUM(BlueprintType)
enum class EVoiceType : uint8
{
    PushToTalk UMETA(DisplayName = "Push To Talk"),
    OpenMic UMETA(DisplayName = "Open Mic"),
    VoiceActivation UMETA(DisplayName = "Voice Activation"),
};

UCLASS()
class HORRORGAME_API UVoiceWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UVoiceWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Voice Settings Management
    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void UpdateVoiceSettings();

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void UpdateVoiceButtonText();

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void RefreshValues();

    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void ResetToDefaults();

    // Voice Testing
    UFUNCTION(BlueprintCallable, Category = "Voice Testing")
    void StartVoiceTest();

    UFUNCTION(BlueprintCallable, Category = "Voice Testing")
    void StopVoiceTest();

    // Proximity Voice
    UFUNCTION(BlueprintCallable, Category = "Voice Settings")
    void UpdateProximitySettings();

protected:
    // Basic Voice Controls
    UPROPERTY(meta = (BindWidget))
    class UCheckBox* VoiceMuteCheckBox;

    UPROPERTY(meta = (BindWidget))
    class USlider* VoiceVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VoiceVolumeValueText;

    UPROPERTY(meta = (BindWidget))
    class UComboBoxString* VoiceTypeComboBox;

    UPROPERTY(meta = (BindWidget))
    class UButton* VoiceTypeButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* TestVoiceButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VoiceTypeButtonText;

    // Advanced Voice Controls
    UPROPERTY(meta = (BindWidget))
    class USlider* VoiceThresholdSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VoiceThresholdValueText;

    UPROPERTY(meta = (BindWidget))
    class USlider* VoiceGainSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VoiceGainValueText;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* NoiseGateCheckBox;

    UPROPERTY(meta = (BindWidget))
    class USlider* NoiseGateThresholdSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* NoiseGateThresholdValueText;

    // Voice Effects
    UPROPERTY(meta = (BindWidget))
    class UCheckBox* VoiceEchoCheckBox;

    UPROPERTY(meta = (BindWidget))
    class USlider* VoiceEchoStrengthSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VoiceEchoStrengthValueText;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* VoiceDistortionCheckBox;

    UPROPERTY(meta = (BindWidget))
    class USlider* VoiceDistortionIntensitySlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VoiceDistortionIntensityValueText;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* WhisperModeCheckBox;

    // Proximity Voice Settings
    UPROPERTY(meta = (BindWidget))
    class UCheckBox* ProximityVoiceCheckBox;

    UPROPERTY(meta = (BindWidget))
    class USlider* ProximityVoiceRangeSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ProximityVoiceRangeValueText;

    // Voice Activity Indicators
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* VoiceActivityBar;

    UPROPERTY(meta = (BindWidget))
    class UImage* MicStatusImage;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* MicStatusText;

    // Push to Talk Key Binding
    UPROPERTY(meta = (BindWidget))
    class UButton* PushToTalkKeyButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* PushToTalkKeyText;

    // Device Selection
    UPROPERTY(meta = (BindWidget))
    class UComboBoxString* InputDeviceComboBox;

    // Speech to Text
    UPROPERTY(meta = (BindWidget))
    class UCheckBox* SpeechToTextCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UComboBoxString* SpeechLanguageComboBox;

private:
    // Event Handlers
    UFUNCTION()
    void OnVoiceMuteChanged(bool bIsChecked);

    UFUNCTION()
    void OnVoiceVolumeChanged(float Value);

    UFUNCTION()
    void OnVoiceTypeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnVoiceThresholdChanged(float Value);

    UFUNCTION()
    void OnVoiceGainChanged(float Value);

    UFUNCTION()
    void OnNoiseGateChanged(bool bIsChecked);

    UFUNCTION()
    void OnNoiseGateThresholdChanged(float Value);

    UFUNCTION()
    void OnVoiceEchoChanged(bool bIsChecked);

    UFUNCTION()
    void OnVoiceEchoStrengthChanged(float Value);

    UFUNCTION()
    void OnVoiceDistortionChanged(bool bIsChecked);

    UFUNCTION()
    void OnVoiceDistortionIntensityChanged(float Value);

    UFUNCTION()
    void OnWhisperModeChanged(bool bIsChecked);

    UFUNCTION()
    void OnProximityVoiceChanged(bool bIsChecked);

    UFUNCTION()
    void OnProximityVoiceRangeChanged(float Value);

    UFUNCTION()
    void OnTestVoiceClicked();

    UFUNCTION()
    void OnPushToTalkKeyClicked();

    UFUNCTION()
    void OnInputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnSpeechToTextChanged(bool bIsChecked);

    UFUNCTION()
    void OnSpeechLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // Helper Functions
    EVoiceType GetCurrentVoiceType() const;
    void SetCurrentVoiceType(EVoiceType NewType);
    void UpdateVoiceActivityIndicator();
    void UpdateMicrophoneStatus();
    void PopulateInputDevices();
    void PopulateSpeechLanguages();
    void BindToSettingsEvents();
    void UnbindFromSettingsEvents();

    // Settings Event Handlers
    UFUNCTION()
    void OnGameSettingsVoiceTypeChanged(int32 NewType);

    UFUNCTION()
    void OnGameSettingsVoiceThresholdChanged(float NewThreshold);

    UFUNCTION()
    void OnGameSettingsPushToTalkKeyChanged(FKey NewKey);

    UFUNCTION()
    void OnGameSettingsProximityVoiceChanged(bool bEnabled);

    UFUNCTION()
    void OnGameSettingsVoiceChanged(bool bEnabled);

    // Internal State
    bool bIsTestingVoice;
    bool bIsListeningForKey;
    float CurrentVoiceActivity;
    FTimerHandle VoiceTestTimer;

    // Cached settings reference
    UPROPERTY()
    UHorrorGameSettings* GameSettings;
};