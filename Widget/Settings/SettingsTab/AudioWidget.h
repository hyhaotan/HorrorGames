#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "AudioWidget.generated.h"

class UHorrorGameSettings;
class USoundMix;

UCLASS()
class HORRORGAME_API UAudioWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UAudioWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    // UI Components
    UPROPERTY(meta = (BindWidget))
    class USlider* MasterVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* MusicVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* SFXVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* VoiceVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* AmbientVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* MasterVolumeText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* MusicVolumeText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SFXVolumeText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VoiceVolumeText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* AmbientVolumeText;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* MasterMuteCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* MusicMuteCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* SFXMuteCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* VoiceMuteCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* AmbientMuteCheckBox;

    // Audio Quality Settings
    UPROPERTY(meta = (BindWidget))
    class USelectionWidget* AudioQualitySelection;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* SpatialAudioCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* AudioCompressionCheckBox;

    // Sound Mix References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundMix* MainSoundMix;
private:
    // Settings Reference
    UPROPERTY()
    UHorrorGameSettings* HorrorGameSettings;

    // Cached Values
    float MasterVolume;
    float MusicVolume;
    float SFXVolume;
    float VoiceVolume;
    float AmbientVolume;

    bool bMasterMuted;
    bool bMusicMuted;
    bool bSFXMuted;
    bool bVoiceMuted;
    bool bAmbientMuted;

    // Event Handlers
    UFUNCTION()
    void OnMasterVolumeChanged(float Value);

    UFUNCTION()
    void OnMusicVolumeChanged(float Value);

    UFUNCTION()
    void OnSFXVolumeChanged(float Value);

    UFUNCTION()
    void OnVoiceVolumeChanged(float Value);

    UFUNCTION()
    void OnAmbientVolumeChanged(float Value);

    UFUNCTION()
    void OnMasterMuteChanged(bool bIsChecked);

    UFUNCTION()
    void OnMusicMuteChanged(bool bIsChecked);

    UFUNCTION()
    void OnSFXMuteChanged(bool bIsChecked);

    UFUNCTION()
    void OnVoiceMuteChanged(bool bIsChecked);

    UFUNCTION()
    void OnAmbientMuteChanged(bool bIsChecked);

    UFUNCTION()
    void OnSpatialAudioChanged(bool bIsChecked);

    UFUNCTION()
    void OnAudioCompressionChanged(bool bIsChecked);

    // Helper Functions
    void UpdateVolumeText(UTextBlock* TextWidget, float Value);
    void ApplyVolumeToAudioSystem();
    void InitializeAudioQuality();

public:
    // Required by MenuSettingSystemWidget
    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void RefreshValues();

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void ResetToDefaults();

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Audio Settings")
    void OnAudioSettingsChanged();
};