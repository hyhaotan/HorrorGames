// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AudioWidget.generated.h"

/**
 * Audio Widget for controlling game audio settings.
 */
UCLASS()
class HORRORGAME_API UAudioWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	// Initializes the widget
	virtual void NativeConstruct() override;

protected:
	// UI bindings for sliders/buttons (can be set in Blueprints)
	UPROPERTY(meta = (BindWidget))
	class USlider* MasterVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	class USlider* MusicVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	class USlider* SFXVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	class UButton* MuteButton;

	// Current audio settings
	float MasterVolume = 1.0f;
	float MusicVolume = 1.0f;
	float SFXVolume = 1.0f;
	bool bIsMuted = false;

	// Functions to handle UI changes
	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

	UFUNCTION()
	void OnMusicVolumeChanged(float Value);

	UFUNCTION()
	void OnSFXVolumeChanged(float Value);

	UFUNCTION()
	void OnMuteButtonClicked();
};
