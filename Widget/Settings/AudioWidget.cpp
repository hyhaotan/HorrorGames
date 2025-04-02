// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Settings/AudioWidget.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"

void UAudioWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind slider changes to functions
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnMasterVolumeChanged);
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnMusicVolumeChanged);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UAudioWidget::OnSFXVolumeChanged);
	}

	// Bind mute button
	if (MuteButton)
	{
		MuteButton->OnClicked.AddDynamic(this, &UAudioWidget::OnMuteButtonClicked);
	}
}

void UAudioWidget::OnMasterVolumeChanged(float Value)
{
	MasterVolume = Value;
	UGameplayStatics::SetSoundMixClassOverride(
		this,
		nullptr,
		nullptr,
		MasterVolume,
		1.0f,
		0.1f
	);
}

void UAudioWidget::OnMusicVolumeChanged(float Value)
{
	MusicVolume = Value;
	// Assuming you have a Music SoundClass
	USoundClass* MusicSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/Audio/MusicSoundClass.MusicSoundClass"));
	if (MusicSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			this,
			nullptr,
			MusicSoundClass,
			MusicVolume,
			1.0f,
			0.1f
		);
	}
}

void UAudioWidget::OnSFXVolumeChanged(float Value)
{
	SFXVolume = Value;
	// Assuming you have a SFX SoundClass
	USoundClass* SFXSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/Audio/SFXSoundClass.SFXSoundClass"));
	if (SFXSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			this,
			nullptr,
			SFXSoundClass,
			SFXVolume,
			1.0f,
			0.1f
		);
	}
}

void UAudioWidget::OnMuteButtonClicked()
{
	bIsMuted = !bIsMuted;

	float NewVolume = bIsMuted ? 0.0f : 1.0f;
	UGameplayStatics::SetSoundMixClassOverride(
		this,
		nullptr,
		nullptr,
		NewVolume,
		1.0f,
		0.1f
	);

	// Update the UI or show a message
	// For example, change button text/icon to indicate mute state
}
