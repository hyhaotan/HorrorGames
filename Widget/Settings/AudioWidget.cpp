#include "HorrorGame/Widget/Settings/AudioWidget.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Sound/SoundMix.h"

void UAudioWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get settings reference
    if (UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get())
    {
        // Initialize all volume sliders
        if (MasterVolumeSlider)
        {
            MasterVolumeSlider->SetValue(GameSettings->MasterVolume);
            MasterVolumeSlider->OnValueChanged.Clear();
            MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnMasterVolumeChanged);
        }

        if (MusicVolumeSlider)
        {
            MusicVolumeSlider->SetValue(GameSettings->MusicVolume);
            MusicVolumeSlider->OnValueChanged.Clear();
            MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnMusicVolumeChanged);
        }

        if (SFXVolumeSlider)
        {
            SFXVolumeSlider->SetValue(GameSettings->SFXVolume);
            SFXVolumeSlider->OnValueChanged.Clear();
            SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnSFXVolumeChanged);
        }

        // Add Voice and Ambient volume sliders if they exist
        // Initialize mute button
        if (MuteButton)
        {
            MuteButton->OnClicked.Clear();
            MuteButton->OnClicked.AddDynamic(this, &UAudioWidget::OnMuteButtonClicked);
        }

        // Set initial values
        MasterVolume = GameSettings->MasterVolume;
        MusicVolume = GameSettings->MusicVolume;
        SFXVolume = GameSettings->SFXVolume;
        bIsMuted = GameSettings->bMuted;
    }
}

void UAudioWidget::OnMasterVolumeChanged(float Value)
{
    MasterVolume = Value;

    if (UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get())
    {
        GameSettings->MasterVolume = Value;
        GameSettings->SaveConfig();

        // Apply to audio system
        UAudioMixerBlueprintLibrary::SetBypassSourceEffectChainEntry(this, nullptr, 0, !bIsMuted);
    }
}

void UAudioWidget::OnMusicVolumeChanged(float Value)
{
    MusicVolume = Value;

    if (UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get())
    {
        GameSettings->MusicVolume = Value;
        GameSettings->SaveConfig();

        // Apply music volume to sound mix
        // TODO: Apply to music sound class
    }
}

void UAudioWidget::OnSFXVolumeChanged(float Value)
{
    SFXVolume = Value;

    if (UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get())
    {
        GameSettings->SFXVolume = Value;
        GameSettings->SaveConfig();

        // Apply SFX volume to sound mix
        // TODO: Apply to SFX sound class
    }
}

void UAudioWidget::OnMuteButtonClicked()
{
    bIsMuted = !bIsMuted;

    if (UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get())
    {
        GameSettings->bMuted = bIsMuted;
        GameSettings->SaveConfig();

        // Mute/unmute all audio
        if (bIsMuted)
        {
            // Mute all audio
            UAudioMixerBlueprintLibrary::SetBypassSourceEffectChainEntry(this, nullptr, 0, false);
        }
        else
        {
            // Restore audio
            UAudioMixerBlueprintLibrary::SetBypassSourceEffectChainEntry(this, nullptr, 0, true);
        }
    }

    // Update button text
    if (MuteButton)
    {
        // TODO: Update button appearance based on mute state
    }
}