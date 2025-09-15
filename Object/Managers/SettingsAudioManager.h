#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sound/SoundCue.h"
#include "Animation/WidgetAnimation.h"
#include "HorrorGame/Data/SettingsStructs.h"
#include "SettingsAudioManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class HORRORGAME_API USettingsAudioManager : public UObject
{
    GENERATED_BODY()

public:
    USettingsAudioManager();

    // === AUDIO MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Settings Audio")
    void PlaySound(ESoundTypes SoundType);

    UFUNCTION(BlueprintCallable, Category = "Settings Audio")
    void SetSoundCues(USoundCue* TabSwitch, USoundCue* Hover, USoundCue* Apply,
        USoundCue* Reset, USoundCue* Back, USoundCue* Error, USoundCue* SettingsChanged);

    UFUNCTION(BlueprintCallable, Category = "Settings Audio")
    void SetSoundCue(ESoundTypes SoundType, USoundCue* SoundCue);

    UFUNCTION(BlueprintCallable, Category = "Settings Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Settings Audio")
    void SetAudioEnabled(bool bEnabled);

    // === ANIMATION MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Settings Animation")
    void PlayTabTransition();

    UFUNCTION(BlueprintCallable, Category = "Settings Animation")
    void PlayFadeIn();

    UFUNCTION(BlueprintCallable, Category = "Settings Animation")
    void PlayFadeOut();

    UFUNCTION(BlueprintCallable, Category = "Settings Animation")
    void PlayButtonHover();

    UFUNCTION(BlueprintCallable, Category = "Settings Animation")
    void SetAnimations(UWidgetAnimation* FadeIn, UWidgetAnimation* FadeOut,
        UWidgetAnimation* TabTransition, UWidgetAnimation* ButtonHover);

    UFUNCTION(BlueprintCallable, Category = "Settings Animation")
    void SetParentWidget(UUserWidget* InParentWidget);

    // === GETTERS ===

    UFUNCTION(BlueprintPure, Category = "Settings Audio")
    USoundCue* GetSoundCue(ESoundTypes SoundType) const;

    UFUNCTION(BlueprintPure, Category = "Settings Audio")
    bool IsAudioEnabled() const { return bAudioEnabled; }

    UFUNCTION(BlueprintPure, Category = "Settings Audio")
    float GetMasterVolume() const { return MasterVolume; }

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "Settings Animation")
    FOnSettingsAppliedDelegate OnFadeOutCompleted;

protected:
    // === AUDIO PROPERTIES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* TabSwitchSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* HoverSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* ApplySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* ResetSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* BackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* ErrorSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* SettingsChangedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bAudioEnabled;

    // === ANIMATION PROPERTIES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UWidgetAnimation* FadeInAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UWidgetAnimation* FadeOutAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UWidgetAnimation* TabTransitionAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UWidgetAnimation* ButtonHoverAnimation;

    // === REFERENCES ===

    UPROPERTY()
    TWeakObjectPtr<UUserWidget> ParentWidget;

private:
    // === HELPER FUNCTIONS ===
    void PlaySoundInternal(USoundCue* SoundCue);

    UFUNCTION()
    void OnFadeOutAnimationFinished();

public:
    // === DEBUG FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugTestAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintAudioConfig() const;
};