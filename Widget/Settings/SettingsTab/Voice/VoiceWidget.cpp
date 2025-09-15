#include "VoiceWidget.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "TimerManager.h"

UVoiceWidget::UVoiceWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsTestingVoice = false;
    bIsListeningForKey = false;
    CurrentVoiceActivity = 0.0f;
    GameSettings = nullptr;
}

void UVoiceWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache game settings
    GameSettings = UHorrorGameSettings::Get();

    // Populate combo boxes
    PopulateInputDevices();
    PopulateSpeechLanguages();

    // Bind events
    BindToSettingsEvents();

    // Initialize voice type combo box
    if (VoiceTypeComboBox)
    {
        VoiceTypeComboBox->ClearOptions();
        VoiceTypeComboBox->AddOption(TEXT("Push To Talk"));
        VoiceTypeComboBox->AddOption(TEXT("Open Mic"));
        VoiceTypeComboBox->AddOption(TEXT("Voice Activation"));
    }

    // Bind UI events
    if (VoiceMuteCheckBox)
    {
        VoiceMuteCheckBox->OnCheckStateChanged.AddDynamic(this, &UVoiceWidget::OnVoiceMuteChanged);
    }

    if (VoiceVolumeSlider)
    {
        VoiceVolumeSlider->OnValueChanged.AddDynamic(this, &UVoiceWidget::OnVoiceVolumeChanged);
    }

    if (VoiceTypeComboBox)
    {
        VoiceTypeComboBox->OnSelectionChanged.AddDynamic(this, &UVoiceWidget::OnVoiceTypeChanged);
    }

    if (VoiceThresholdSlider)
    {
        VoiceThresholdSlider->OnValueChanged.AddDynamic(this, &UVoiceWidget::OnVoiceThresholdChanged);
    }

    if (VoiceGainSlider)
    {
        VoiceGainSlider->OnValueChanged.AddDynamic(this, &UVoiceWidget::OnVoiceGainChanged);
    }

    if (NoiseGateCheckBox)
    {
        NoiseGateCheckBox->OnCheckStateChanged.AddDynamic(this, &UVoiceWidget::OnNoiseGateChanged);
    }

    if (NoiseGateThresholdSlider)
    {
        NoiseGateThresholdSlider->OnValueChanged.AddDynamic(this, &UVoiceWidget::OnNoiseGateThresholdChanged);
    }

    if (VoiceEchoCheckBox)
    {
        VoiceEchoCheckBox->OnCheckStateChanged.AddDynamic(this, &UVoiceWidget::OnVoiceEchoChanged);
    }

    if (VoiceEchoStrengthSlider)
    {
        VoiceEchoStrengthSlider->OnValueChanged.AddDynamic(this, &UVoiceWidget::OnVoiceEchoStrengthChanged);
    }

    if (VoiceDistortionCheckBox)
    {
        VoiceDistortionCheckBox->OnCheckStateChanged.AddDynamic(this, &UVoiceWidget::OnVoiceDistortionChanged);
    }

    if (VoiceDistortionIntensitySlider)
    {
        VoiceDistortionIntensitySlider->OnValueChanged.AddDynamic(this, &UVoiceWidget::OnVoiceDistortionIntensityChanged);
    }

    if (WhisperModeCheckBox)
    {
        WhisperModeCheckBox->OnCheckStateChanged.AddDynamic(this, &UVoiceWidget::OnWhisperModeChanged);
    }

    if (ProximityVoiceCheckBox)
    {
        ProximityVoiceCheckBox->OnCheckStateChanged.AddDynamic(this, &UVoiceWidget::OnProximityVoiceChanged);
    }

    if (ProximityVoiceRangeSlider)
    {
        ProximityVoiceRangeSlider->OnValueChanged.AddDynamic(this, &UVoiceWidget::OnProximityVoiceRangeChanged);
    }

    if (TestVoiceButton)
    {
        TestVoiceButton->OnClicked.AddDynamic(this, &UVoiceWidget::OnTestVoiceClicked);
    }

    if (PushToTalkKeyButton)
    {
        PushToTalkKeyButton->OnClicked.AddDynamic(this, &UVoiceWidget::OnPushToTalkKeyClicked);
    }

    if (InputDeviceComboBox)
    {
        InputDeviceComboBox->OnSelectionChanged.AddDynamic(this, &UVoiceWidget::OnInputDeviceChanged);
    }

    if (SpeechToTextCheckBox)
    {
        SpeechToTextCheckBox->OnCheckStateChanged.AddDynamic(this, &UVoiceWidget::OnSpeechToTextChanged);
    }

    if (SpeechLanguageComboBox)
    {
        SpeechLanguageComboBox->OnSelectionChanged.AddDynamic(this, &UVoiceWidget::OnSpeechLanguageChanged);
    }

    // Refresh UI with current values
    RefreshValues();
}

void UVoiceWidget::NativeDestruct()
{
    UnbindFromSettingsEvents();

    if (bIsTestingVoice)
    {
        StopVoiceTest();
    }

    Super::NativeDestruct();
}

void UVoiceWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateVoiceActivityIndicator();
    UpdateMicrophoneStatus();
}

void UVoiceWidget::UpdateVoiceSettings()
{
    if (!GameSettings)
    {
        return;
    }

    // Update all voice-related settings
    // This would typically interface with your voice chat system
    // For example, updating voice codec settings, audio processing, etc.

    UE_LOG(LogTemp, Log, TEXT("Voice settings updated"));
}

void UVoiceWidget::UpdateVoiceButtonText()
{
    if (!VoiceTypeButtonText || !GameSettings)
    {
        return;
    }

    EVoiceType CurrentType = GetCurrentVoiceType();
    FString ButtonText;

    switch (CurrentType)
    {
    case EVoiceType::PushToTalk:
        ButtonText = FString::Printf(TEXT("Push to Talk (%s)"), *GameSettings->GetPushToTalkKey().ToString());
        break;
    case EVoiceType::OpenMic:
        ButtonText = TEXT("Open Mic");
        break;
    case EVoiceType::VoiceActivation:
        ButtonText = TEXT("Voice Activation");
        break;
    }

    VoiceTypeButtonText->SetText(FText::FromString(ButtonText));
}

void UVoiceWidget::RefreshValues()
{
    if (!GameSettings)
    {
        return;
    }

    // Basic voice settings
    if (VoiceMuteCheckBox)
    {
        VoiceMuteCheckBox->SetIsChecked(GameSettings->GetVoiceMuted());
    }

    if (VoiceVolumeSlider && VoiceVolumeValueText)
    {
        float Volume = GameSettings->GetVoiceVolume();
        VoiceVolumeSlider->SetValue(Volume);
        VoiceVolumeValueText->SetText(FText::AsNumber(FMath::RoundToInt(Volume * 100)));
    }

    if (VoiceTypeComboBox)
    {
        VoiceTypeComboBox->SetSelectedIndex(GameSettings->GetVoiceType());
    }

    // Advanced voice settings
    if (VoiceThresholdSlider && VoiceThresholdValueText)
    {
        float Threshold = GameSettings->GetVoiceThreshold();
        VoiceThresholdSlider->SetValue(Threshold);
        VoiceThresholdValueText->SetText(FText::AsNumber(FMath::RoundToInt(Threshold * 100)));
    }

    if (VoiceGainSlider && VoiceGainValueText)
    {
        float Gain = GameSettings->VoiceGain;
        VoiceGainSlider->SetValue(Gain);
        VoiceGainValueText->SetText(FText::AsNumber(FMath::RoundToInt(Gain * 100)));
    }

    if (NoiseGateCheckBox)
    {
        NoiseGateCheckBox->SetIsChecked(GameSettings->bVoiceNoiseGate);
    }

    if (NoiseGateThresholdSlider && NoiseGateThresholdValueText)
    {
        float NoiseThreshold = GameSettings->NoiseGateThreshold;
        NoiseGateThresholdSlider->SetValue(NoiseThreshold);
        NoiseGateThresholdValueText->SetText(FText::AsNumber(FMath::RoundToInt(NoiseThreshold * 100)));
    }

    // Voice effects
    if (VoiceEchoCheckBox)
    {
        VoiceEchoCheckBox->SetIsChecked(GameSettings->bVoiceEcho);
    }

    if (VoiceEchoStrengthSlider && VoiceEchoStrengthValueText)
    {
        float EchoStrength = GameSettings->VoiceEchoStrength;
        VoiceEchoStrengthSlider->SetValue(EchoStrength);
        VoiceEchoStrengthValueText->SetText(FText::AsNumber(FMath::RoundToInt(EchoStrength * 100)));
    }

    if (VoiceDistortionCheckBox)
    {
        VoiceDistortionCheckBox->SetIsChecked(GameSettings->GetVoiceDistortion());
    }

    if (VoiceDistortionIntensitySlider && VoiceDistortionIntensityValueText)
    {
        float DistortionIntensity = GameSettings->VoiceDistortionIntensity;
        VoiceDistortionIntensitySlider->SetValue(DistortionIntensity);
        VoiceDistortionIntensityValueText->SetText(FText::AsNumber(FMath::RoundToInt(DistortionIntensity * 100)));
    }

    if (WhisperModeCheckBox)
    {
        WhisperModeCheckBox->SetIsChecked(GameSettings->GetWhisperMode());
    }

    // Proximity voice
    if (ProximityVoiceCheckBox)
    {
        ProximityVoiceCheckBox->SetIsChecked(GameSettings->GetProximityVoice());
    }

    if (ProximityVoiceRangeSlider && ProximityVoiceRangeValueText)
    {
        float Range = GameSettings->ProximityVoiceRange;
        ProximityVoiceRangeSlider->SetValue(Range);
        ProximityVoiceRangeValueText->SetText(FText::AsNumber(FMath::RoundToInt(Range)));
    }

    // Push to talk key
    if (PushToTalkKeyText)
    {
        PushToTalkKeyText->SetText(FText::FromString(GameSettings->GetPushToTalkKey().ToString()));
    }

    // Speech to text
    if (SpeechToTextCheckBox)
    {
        SpeechToTextCheckBox->SetIsChecked(GameSettings->bSpeechToText);
    }

    if (SpeechLanguageComboBox)
    {
        SpeechLanguageComboBox->SetSelectedOption(GameSettings->SpeechLanguage);
    }

    UpdateVoiceButtonText();
}

void UVoiceWidget::ResetToDefaults()
{
    if (!GameSettings)
    {
        return;
    }

    // Reset voice settings to defaults
    GameSettings->SetVoiceMuted(false);
    GameSettings->SetVoiceVolume(0.8f);
    GameSettings->SetVoiceType(0);
    GameSettings->SetVoiceThreshold(0.3f);
    GameSettings->SetPushToTalkKey(EKeys::V);
    GameSettings->SetProximityVoice(true);
    GameSettings->SetWhisperMode(false);
    GameSettings->SetVoiceDistortion(false);

    // Refresh UI
    RefreshValues();
}

void UVoiceWidget::StartVoiceTest()
{
    if (bIsTestingVoice)
    {
        return;
    }

    bIsTestingVoice = true;

    if (TestVoiceButton)
    {
        TestVoiceButton->SetIsEnabled(false);
    }

    // Start voice test timer (simulate testing for 5 seconds)
    GetWorld()->GetTimerManager().SetTimer(VoiceTestTimer, this, &UVoiceWidget::StopVoiceTest, 5.0f, false);

    // Here you would start recording/testing the microphone
    UE_LOG(LogTemp, Log, TEXT("Voice test started"));
}

void UVoiceWidget::StopVoiceTest()
{
    if (!bIsTestingVoice)
    {
        return;
    }

    bIsTestingVoice = false;

    if (TestVoiceButton)
    {
        TestVoiceButton->SetIsEnabled(true);
    }

    GetWorld()->GetTimerManager().ClearTimer(VoiceTestTimer);

    // Here you would stop the microphone test
    UE_LOG(LogTemp, Log, TEXT("Voice test stopped"));
}

void UVoiceWidget::UpdateProximitySettings()
{
    if (!GameSettings)
    {
        return;
    }

    // Update proximity voice settings
    // This would interface with your voice chat system to update proximity ranges
    UE_LOG(LogTemp, Log, TEXT("Proximity voice settings updated"));
}

// Event Handlers
void UVoiceWidget::OnVoiceMuteChanged(bool bIsChecked)
{
    if (GameSettings)
    {
        GameSettings->SetVoiceMuted(bIsChecked);
    }
}

void UVoiceWidget::OnVoiceVolumeChanged(float Value)
{
    if (GameSettings)
    {
        GameSettings->SetVoiceVolume(Value);
    }

    if (VoiceVolumeValueText)
    {
        VoiceVolumeValueText->SetText(FText::AsNumber(FMath::RoundToInt(Value * 100)));
    }
}

void UVoiceWidget::OnVoiceTypeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (SelectionType == ESelectInfo::Type::OnMouseClick || SelectionType == ESelectInfo::Type::OnKeyPress)
    {
        int32 SelectedIndex = 0;
        if (VoiceTypeComboBox)
        {
            SelectedIndex = VoiceTypeComboBox->GetSelectedIndex();
        }

        if (GameSettings)
        {
            GameSettings->SetVoiceType(SelectedIndex);
        }

        UpdateVoiceButtonText();
    }
}

void UVoiceWidget::OnVoiceThresholdChanged(float Value)
{
    if (GameSettings)
    {
        GameSettings->SetVoiceThreshold(Value);
    }

    if (VoiceThresholdValueText)
    {
        VoiceThresholdValueText->SetText(FText::AsNumber(FMath::RoundToInt(Value * 100)));
    }
}

void UVoiceWidget::OnVoiceGainChanged(float Value)
{
    if (GameSettings)
    {
        GameSettings->VoiceGain = Value;
        GameSettings->SaveConfig();
    }

    if (VoiceGainValueText)
    {
        VoiceGainValueText->SetText(FText::AsNumber(FMath::RoundToInt(Value * 100)));
    }
}

void UVoiceWidget::OnNoiseGateChanged(bool bIsChecked)
{
    if (GameSettings)
    {
        GameSettings->bVoiceNoiseGate = bIsChecked;
        GameSettings->SaveConfig();
    }
}

void UVoiceWidget::OnNoiseGateThresholdChanged(float Value)
{
    if (GameSettings)
    {
        GameSettings->NoiseGateThreshold = Value;
        GameSettings->SaveConfig();
    }

    if (NoiseGateThresholdValueText)
    {
        NoiseGateThresholdValueText->SetText(FText::AsNumber(FMath::RoundToInt(Value * 100)));
    }
}

void UVoiceWidget::OnVoiceEchoChanged(bool bIsChecked)
{
    if (GameSettings)
    {
        GameSettings->bVoiceEcho = bIsChecked;
        GameSettings->SaveConfig();
    }
}

void UVoiceWidget::OnVoiceEchoStrengthChanged(float Value)
{
    if (GameSettings)
    {
        GameSettings->VoiceEchoStrength = Value;
        GameSettings->SaveConfig();
    }

    if (VoiceEchoStrengthValueText)
    {
        VoiceEchoStrengthValueText->SetText(FText::AsNumber(FMath::RoundToInt(Value * 100)));
    }
}

void UVoiceWidget::OnVoiceDistortionChanged(bool bIsChecked)
{
    if (GameSettings)
    {
        GameSettings->SetVoiceDistortion(bIsChecked);
    }
}

void UVoiceWidget::OnVoiceDistortionIntensityChanged(float Value)
{
    if (GameSettings)
    {
        GameSettings->VoiceDistortionIntensity = Value;
        GameSettings->SaveConfig();
    }

    if (VoiceDistortionIntensityValueText)
    {
        VoiceDistortionIntensityValueText->SetText(FText::AsNumber(FMath::RoundToInt(Value * 100)));
    }
}

void UVoiceWidget::OnWhisperModeChanged(bool bIsChecked)
{
    if (GameSettings)
    {
        GameSettings->SetWhisperMode(bIsChecked);
    }
}

void UVoiceWidget::OnProximityVoiceChanged(bool bIsChecked)
{
    if (GameSettings)
    {
        GameSettings->SetProximityVoice(bIsChecked);
    }
}

void UVoiceWidget::OnProximityVoiceRangeChanged(float Value)
{
    if (GameSettings)
    {
        GameSettings->ProximityVoiceRange = Value;
        GameSettings->SaveConfig();
    }

    if (ProximityVoiceRangeValueText)
    {
        ProximityVoiceRangeValueText->SetText(FText::AsNumber(FMath::RoundToInt(Value)));
    }
}

void UVoiceWidget::OnTestVoiceClicked()
{
    if (bIsTestingVoice)
    {
        StopVoiceTest();
    }
    else
    {
        StartVoiceTest();
    }
}

void UVoiceWidget::OnPushToTalkKeyClicked()
{
    if (bIsListeningForKey)
    {
        return;
    }

    bIsListeningForKey = true;

    if (PushToTalkKeyText)
    {
        PushToTalkKeyText->SetText(FText::FromString(TEXT("Press any key...")));
    }

    // Here you would implement key listening logic
    // For now, we'll just simulate it
    GetWorld()->GetTimerManager().SetTimer(VoiceTestTimer, [this]()
        {
            bIsListeningForKey = false;
            if (GameSettings)
            {
                GameSettings->SetPushToTalkKey(EKeys::T); // Example: set to T key
            }
            RefreshValues();
        }, 3.0f, false);
}

void UVoiceWidget::OnInputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (SelectionType == ESelectInfo::Type::OnMouseClick || SelectionType == ESelectInfo::Type::OnKeyPress)
    {
        if (GameSettings)
        {
            GameSettings->PreferredInputDevice = SelectedItem;
            GameSettings->SaveConfig();
        }
    }
}

void UVoiceWidget::OnSpeechToTextChanged(bool bIsChecked)
{
    if (GameSettings)
    {
        GameSettings->bSpeechToText = bIsChecked;
        GameSettings->SaveConfig();
    }
}

void UVoiceWidget::OnSpeechLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (SelectionType == ESelectInfo::Type::OnMouseClick || SelectionType == ESelectInfo::Type::OnKeyPress)
    {
        if (GameSettings)
        {
            GameSettings->SpeechLanguage = SelectedItem;
            GameSettings->SaveConfig();
        }
    }
}

// Helper Functions
EVoiceType UVoiceWidget::GetCurrentVoiceType() const
{
    if (!GameSettings)
    {
        return EVoiceType::PushToTalk;
    }

    return static_cast<EVoiceType>(GameSettings->GetVoiceType());
}

void UVoiceWidget::SetCurrentVoiceType(EVoiceType NewType)
{
    if (GameSettings)
    {
        GameSettings->SetVoiceType(static_cast<int32>(NewType));
    }
}

void UVoiceWidget::UpdateVoiceActivityIndicator()
{
    if (!VoiceActivityBar)
    {
        return;
    }

    // Simulate voice activity (in real implementation, this would come from microphone input)
    if (bIsTestingVoice)
    {
        CurrentVoiceActivity = FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f) * 0.5f + 0.5f;
    }
    else
    {
        CurrentVoiceActivity = FMath::FInterpTo(CurrentVoiceActivity, 0.0f, GetWorld()->GetDeltaSeconds(), 5.0f);
    }

    VoiceActivityBar->SetPercent(CurrentVoiceActivity);
}

void UVoiceWidget::UpdateMicrophoneStatus()
{
    if (!MicStatusText || !GameSettings)
    {
        return;
    }

    FString StatusText;
    FLinearColor StatusColor = FLinearColor::White;

    if (GameSettings->GetVoiceMuted())
    {
        StatusText = TEXT("Muted");
        StatusColor = FLinearColor::Red;
    }
    else
    {
        switch (GetCurrentVoiceType())
        {
        case EVoiceType::PushToTalk:
            StatusText = TEXT("Push to Talk");
            StatusColor = FLinearColor::Yellow;
            break;
        case EVoiceType::OpenMic:
            StatusText = TEXT("Open Mic");
            StatusColor = FLinearColor::Green;
            break;
        case EVoiceType::VoiceActivation:
            StatusText = TEXT("Voice Activation");
            StatusColor = FLinearColor::Blue;
            break;
        }
    }

    MicStatusText->SetText(FText::FromString(StatusText));
    MicStatusText->SetColorAndOpacity(FSlateColor(StatusColor));

    // Update microphone status image
    if (MicStatusImage)
    {
        MicStatusImage->SetColorAndOpacity(StatusColor);
    }
}

void UVoiceWidget::PopulateInputDevices()
{
    if (!InputDeviceComboBox)
    {
        return;
    }

    InputDeviceComboBox->ClearOptions();

    // Add default option
    InputDeviceComboBox->AddOption(TEXT("Default"));

    // In a real implementation, you would enumerate available audio input devices
    // For now, we'll add some example devices
    InputDeviceComboBox->AddOption(TEXT("Built-in Microphone"));
    InputDeviceComboBox->AddOption(TEXT("USB Headset"));
    InputDeviceComboBox->AddOption(TEXT("Wireless Headset"));
    InputDeviceComboBox->AddOption(TEXT("External Microphone"));

    // Set current selection
    if (GameSettings)
    {
        InputDeviceComboBox->SetSelectedOption(GameSettings->PreferredInputDevice);
    }
}

void UVoiceWidget::PopulateSpeechLanguages()
{
    if (!SpeechLanguageComboBox)
    {
        return;
    }

    SpeechLanguageComboBox->ClearOptions();

    // Add supported speech languages
    SpeechLanguageComboBox->AddOption(TEXT("en-US"));
    SpeechLanguageComboBox->AddOption(TEXT("en-GB"));
    SpeechLanguageComboBox->AddOption(TEXT("fr-FR"));
    SpeechLanguageComboBox->AddOption(TEXT("de-DE"));
    SpeechLanguageComboBox->AddOption(TEXT("es-ES"));
    SpeechLanguageComboBox->AddOption(TEXT("it-IT"));
    SpeechLanguageComboBox->AddOption(TEXT("ja-JP"));
    SpeechLanguageComboBox->AddOption(TEXT("ko-KR"));
    SpeechLanguageComboBox->AddOption(TEXT("zh-CN"));
    SpeechLanguageComboBox->AddOption(TEXT("vi-VN")); // Vietnamese

    // Set current selection
    if (GameSettings)
    {
        SpeechLanguageComboBox->SetSelectedOption(GameSettings->SpeechLanguage);
    }
}

void UVoiceWidget::BindToSettingsEvents()
{
    if (!GameSettings)
    {
        return;
    }

    GameSettings->OnVoiceTypeChanged.AddDynamic(this, &UVoiceWidget::OnGameSettingsVoiceTypeChanged);
    GameSettings->OnVoiceThresholdChanged.AddDynamic(this, &UVoiceWidget::OnGameSettingsVoiceThresholdChanged);
    GameSettings->OnPushToTalkKeyChanged.AddDynamic(this, &UVoiceWidget::OnGameSettingsPushToTalkKeyChanged);
    GameSettings->OnProximityVoiceChanged.AddDynamic(this, &UVoiceWidget::OnGameSettingsProximityVoiceChanged);
    GameSettings->OnVoiceChanged.AddDynamic(this, &UVoiceWidget::OnGameSettingsVoiceChanged);
}

void UVoiceWidget::UnbindFromSettingsEvents()
{
    if (!GameSettings)
    {
        return;
    }

    GameSettings->OnVoiceTypeChanged.RemoveDynamic(this, &UVoiceWidget::OnGameSettingsVoiceTypeChanged);
    GameSettings->OnVoiceThresholdChanged.RemoveDynamic(this, &UVoiceWidget::OnGameSettingsVoiceThresholdChanged);
    GameSettings->OnPushToTalkKeyChanged.RemoveDynamic(this, &UVoiceWidget::OnGameSettingsPushToTalkKeyChanged);
    GameSettings->OnProximityVoiceChanged.RemoveDynamic(this, &UVoiceWidget::OnGameSettingsProximityVoiceChanged);
    GameSettings->OnVoiceChanged.RemoveDynamic(this, &UVoiceWidget::OnGameSettingsVoiceChanged);
}

// Settings Event Handlers
void UVoiceWidget::OnGameSettingsVoiceTypeChanged(int32 NewType)
{
    if (VoiceTypeComboBox)
    {
        VoiceTypeComboBox->SetSelectedIndex(NewType);
    }

    UpdateVoiceButtonText();
}

void UVoiceWidget::OnGameSettingsVoiceThresholdChanged(float NewThreshold)
{
    if (VoiceThresholdSlider)
    {
        VoiceThresholdSlider->SetValue(NewThreshold);
    }

    if (VoiceThresholdValueText)
    {
        VoiceThresholdValueText->SetText(FText::AsNumber(FMath::RoundToInt(NewThreshold * 100)));
    }
}

void UVoiceWidget::OnGameSettingsPushToTalkKeyChanged(FKey NewKey)
{
    if (PushToTalkKeyText)
    {
        PushToTalkKeyText->SetText(FText::FromString(NewKey.ToString()));
    }

    UpdateVoiceButtonText();
}

void UVoiceWidget::OnGameSettingsProximityVoiceChanged(bool bEnabled)
{
    if (ProximityVoiceCheckBox)
    {
        ProximityVoiceCheckBox->SetIsChecked(bEnabled);
    }
}

void UVoiceWidget::OnGameSettingsVoiceChanged(bool bEnabled)
{
    RefreshValues();
}