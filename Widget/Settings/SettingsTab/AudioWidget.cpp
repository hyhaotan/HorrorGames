#include "AudioWidget.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "HorrorGame/Widget/Settings/SelectionWidget.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"

UAudioWidget::UAudioWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , HorrorGameSettings(nullptr)
    , MasterVolume(1.0f)
    , MusicVolume(1.0f)
    , SFXVolume(1.0f)
    , VoiceVolume(1.0f)
    , AmbientVolume(1.0f)
    , bMasterMuted(false)
    , bMusicMuted(false)
    , bSFXMuted(false)
    , bVoiceMuted(false)
    , bAmbientMuted(false)
{
}

void UAudioWidget::NativeConstruct()
{
    Super::NativeConstruct();

    HorrorGameSettings = UHorrorGameSettings::Get();

    if (HorrorGameSettings)
    {
        RefreshValues();
    }

    InitializeAudioQuality();
}

UWidget* UAudioWidget::NativeGetDesiredFocusTarget() const
{
    return MasterVolumeSlider ? MasterVolumeSlider : Super::NativeGetDesiredFocusTarget();
}

void UAudioWidget::RefreshValues()
{
    if (!HorrorGameSettings)
        return;

    // Master Volume
    if (MasterVolumeSlider)
    {
        MasterVolume = HorrorGameSettings->MasterVolume;
        MasterVolumeSlider->SetValue(MasterVolume);
        MasterVolumeSlider->OnValueChanged.Clear();
        MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnMasterVolumeChanged);
        UpdateVolumeText(MasterVolumeText, MasterVolume);
    }

    if (MasterMuteCheckBox)
    {
        bMasterMuted = HorrorGameSettings->bMasterMuted;
        MasterMuteCheckBox->SetIsChecked(bMasterMuted);
        MasterMuteCheckBox->OnCheckStateChanged.Clear();
        MasterMuteCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioWidget::OnMasterMuteChanged);
    }

    // Music Volume
    if (MusicVolumeSlider)
    {
        MusicVolume = HorrorGameSettings->MusicVolume;
        MusicVolumeSlider->SetValue(MusicVolume);
        MusicVolumeSlider->OnValueChanged.Clear();
        MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnMusicVolumeChanged);
        UpdateVolumeText(MusicVolumeText, MusicVolume);
    }

    if (MusicMuteCheckBox)
    {
        bMusicMuted = HorrorGameSettings->bMusicMuted;
        MusicMuteCheckBox->SetIsChecked(bMusicMuted);
        MusicMuteCheckBox->OnCheckStateChanged.Clear();
        MusicMuteCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioWidget::OnMusicMuteChanged);
    }

    // SFX Volume
    if (SFXVolumeSlider)
    {
        SFXVolume = HorrorGameSettings->SFXVolume;
        SFXVolumeSlider->SetValue(SFXVolume);
        SFXVolumeSlider->OnValueChanged.Clear();
        SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnSFXVolumeChanged);
        UpdateVolumeText(SFXVolumeText, SFXVolume);
    }

    if (SFXMuteCheckBox)
    {
        bSFXMuted = HorrorGameSettings->bSFXMuted;
        SFXMuteCheckBox->SetIsChecked(bSFXMuted);
        SFXMuteCheckBox->OnCheckStateChanged.Clear();
        SFXMuteCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioWidget::OnSFXMuteChanged);
    }

    // Voice Volume
    if (VoiceVolumeSlider)
    {
        VoiceVolume = HorrorGameSettings->VoiceVolume;
        VoiceVolumeSlider->SetValue(VoiceVolume);
        VoiceVolumeSlider->OnValueChanged.Clear();
        VoiceVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnVoiceVolumeChanged);
        UpdateVolumeText(VoiceVolumeText, VoiceVolume);
    }

    if (VoiceMuteCheckBox)
    {
        bVoiceMuted = HorrorGameSettings->bVoiceMuted;
        VoiceMuteCheckBox->SetIsChecked(bVoiceMuted);
        VoiceMuteCheckBox->OnCheckStateChanged.Clear();
        VoiceMuteCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioWidget::OnVoiceMuteChanged);
    }

    // Ambient Volume
    if (AmbientVolumeSlider)
    {
        AmbientVolume = HorrorGameSettings->AmbientVolume;
        AmbientVolumeSlider->SetValue(AmbientVolume);
        AmbientVolumeSlider->OnValueChanged.Clear();
        AmbientVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnAmbientVolumeChanged);
        UpdateVolumeText(AmbientVolumeText, AmbientVolume);
    }

    if (AmbientMuteCheckBox)
    {
        bAmbientMuted = HorrorGameSettings->bAmbientMuted;
        AmbientMuteCheckBox->SetIsChecked(bAmbientMuted);
        AmbientMuteCheckBox->OnCheckStateChanged.Clear();
        AmbientMuteCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioWidget::OnAmbientMuteChanged);
    }

    // Audio Quality Settings
    if (SpatialAudioCheckBox)
    {
        SpatialAudioCheckBox->SetIsChecked(HorrorGameSettings->bSpatialAudioEnabled);
        SpatialAudioCheckBox->OnCheckStateChanged.Clear();
        SpatialAudioCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioWidget::OnSpatialAudioChanged);
    }

    if (AudioCompressionCheckBox)
    {
        AudioCompressionCheckBox->SetIsChecked(HorrorGameSettings->bAudioCompressionEnabled);
        AudioCompressionCheckBox->OnCheckStateChanged.Clear();
        AudioCompressionCheckBox->OnCheckStateChanged.AddDynamic(this, &UAudioWidget::OnAudioCompressionChanged);
    }
}

void UAudioWidget::ResetToDefaults()
{
    if (!HorrorGameSettings)
        return;

    // Reset all audio settings to defaults
    HorrorGameSettings->MasterVolume = 1.0f;
    HorrorGameSettings->MusicVolume = 0.8f;
    HorrorGameSettings->SFXVolume = 1.0f;
    HorrorGameSettings->VoiceVolume = 1.0f;
    HorrorGameSettings->AmbientVolume = 0.7f;

    HorrorGameSettings->bMasterMuted = false;
    HorrorGameSettings->bMusicMuted = false;
    HorrorGameSettings->bSFXMuted = false;
    HorrorGameSettings->bVoiceMuted = false;
    HorrorGameSettings->bAmbientMuted = false;

    HorrorGameSettings->AudioQuality = 2; // High quality
    HorrorGameSettings->bSpatialAudioEnabled = true;
    HorrorGameSettings->bAudioCompressionEnabled = false;

    // Refresh UI with new values
    RefreshValues();

    // Apply to audio system
    ApplyVolumeToAudioSystem();

    UE_LOG(LogTemp, Log, TEXT("Audio settings reset to defaults"));
}

void UAudioWidget::InitializeAudioQuality()
{
    if (!AudioQualitySelection)
        return;

    AudioQualitySelection->Clear();

    const FString QualityOptions[] = {
        TEXT("Low"),
        TEXT("Medium"),
        TEXT("High"),
        TEXT("Ultra")
    };

    const int32 CurrentQuality = HorrorGameSettings ? HorrorGameSettings->AudioQuality : 2;

    for (int32 i = 0; i < 4; ++i)
    {
        AudioQualitySelection->AddOption({
            FText::FromString(QualityOptions[i])
            });

        if (CurrentQuality == i)
        {
            AudioQualitySelection->SetCurrentSelection(i);
        }
    }

    AudioQualitySelection->OnSelectionChange.BindLambda([this](int32 InSelection)
        {
            if (HorrorGameSettings)
            {
                HorrorGameSettings->AudioQuality = InSelection;
                HorrorGameSettings->SaveConfig();

                // Apply audio quality changes
                ApplyVolumeToAudioSystem();
            }
        });
}

void UAudioWidget::OnMasterVolumeChanged(float Value)
{
    MasterVolume = Value;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->MasterVolume = Value;
        HorrorGameSettings->SaveConfig();
    }

    UpdateVolumeText(MasterVolumeText, Value);
    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnMusicVolumeChanged(float Value)
{
    MusicVolume = Value;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->MusicVolume = Value;
        HorrorGameSettings->SaveConfig();
    }

    UpdateVolumeText(MusicVolumeText, Value);
    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnSFXVolumeChanged(float Value)
{
    SFXVolume = Value;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->SFXVolume = Value;
        HorrorGameSettings->SaveConfig();
    }

    UpdateVolumeText(SFXVolumeText, Value);
    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnVoiceVolumeChanged(float Value)
{
    VoiceVolume = Value;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->VoiceVolume = Value;
        HorrorGameSettings->SaveConfig();
    }

    UpdateVolumeText(VoiceVolumeText, Value);
    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnAmbientVolumeChanged(float Value)
{
    AmbientVolume = Value;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->AmbientVolume = Value;
        HorrorGameSettings->SaveConfig();
    }

    UpdateVolumeText(AmbientVolumeText, Value);
    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnMasterMuteChanged(bool bIsChecked)
{
    bMasterMuted = bIsChecked;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->bMasterMuted = bIsChecked;
        HorrorGameSettings->SaveConfig();
    }

    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnMusicMuteChanged(bool bIsChecked)
{
    bMusicMuted = bIsChecked;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->bMusicMuted = bIsChecked;
        HorrorGameSettings->SaveConfig();
    }

    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnSFXMuteChanged(bool bIsChecked)
{
    bSFXMuted = bIsChecked;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->bSFXMuted = bIsChecked;
        HorrorGameSettings->SaveConfig();
    }

    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnVoiceMuteChanged(bool bIsChecked)
{
    bVoiceMuted = bIsChecked;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->bVoiceMuted = bIsChecked;
        HorrorGameSettings->SaveConfig();
    }

    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnAmbientMuteChanged(bool bIsChecked)
{
    bAmbientMuted = bIsChecked;
    if (HorrorGameSettings)
    {
        HorrorGameSettings->bAmbientMuted = bIsChecked;
        HorrorGameSettings->SaveConfig();
    }

    ApplyVolumeToAudioSystem();
    OnAudioSettingsChanged();
}

void UAudioWidget::OnSpatialAudioChanged(bool bIsChecked)
{
    if (HorrorGameSettings)
    {
        HorrorGameSettings->bSpatialAudioEnabled = bIsChecked;
        HorrorGameSettings->SaveConfig();

        // Apply spatial audio setting
        // TODO: Implement spatial audio toggle
    }
    OnAudioSettingsChanged();
}

void UAudioWidget::OnAudioCompressionChanged(bool bIsChecked)
{
    if (HorrorGameSettings)
    {
        HorrorGameSettings->bAudioCompressionEnabled = bIsChecked;
        HorrorGameSettings->SaveConfig();

        // Apply audio compression setting
        // TODO: Implement audio compression toggle
    }
    OnAudioSettingsChanged();
}

void UAudioWidget::UpdateVolumeText(UTextBlock* TextWidget, float Value)
{
    if (TextWidget)
    {
        int32 Percentage = FMath::RoundToInt(Value * 100.0f);
        TextWidget->SetText(FText::AsNumber(Percentage));
    }
}

void UAudioWidget::ApplyVolumeToAudioSystem()
{
    // Apply master volume
    float FinalMasterVolume = bMasterMuted ? 0.0f : MasterVolume;
    UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MainSoundMix, nullptr, FinalMasterVolume, 1.0f, 0.1f);

    // Apply individual channel volumes
    float FinalMusicVolume = (bMasterMuted || bMusicMuted) ? 0.0f : (MasterVolume * MusicVolume);
    float FinalSFXVolume = (bMasterMuted || bSFXMuted) ? 0.0f : (MasterVolume * SFXVolume);
    float FinalVoiceVolume = (bMasterMuted || bVoiceMuted) ? 0.0f : (MasterVolume * VoiceVolume);
    float FinalAmbientVolume = (bMasterMuted || bAmbientMuted) ? 0.0f : (MasterVolume * AmbientVolume);

    // TODO: Apply to specific sound classes
    // UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MainSoundMix, MusicSoundClass, FinalMusicVolume, 1.0f, 0.1f);
    // UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MainSoundMix, SFXSoundClass, FinalSFXVolume, 1.0f, 0.1f);
    // UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MainSoundMix, VoiceSoundClass, FinalVoiceVolume, 1.0f, 0.1f);
    // UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MainSoundMix, AmbientSoundClass, FinalAmbientVolume, 1.0f, 0.1f);
}