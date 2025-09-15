#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "Engine/World.h"
#include "HorrorGame/GameInstance/VoiceChatManager.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "VoiceInputComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API UVoiceInputComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVoiceInputComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Input Setup
    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void SetupInputBindings(class UInputComponent* InputComponent);

    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void RemoveInputBindings(class UInputComponent* InputComponent);

    // Push to Talk Controls
    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void StartPushToTalk();

    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void StopPushToTalk();

    // Quick Mute Toggle
    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void ToggleMute();

    // Voice Type Cycling
    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void CycleVoiceType();

    // Volume Controls
    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void IncreaseVoiceVolume(float Amount = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void DecreaseVoiceVolume(float Amount = 0.1f);

    // Horror-specific Controls
    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void ActivateWhisperMode();

    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void DeactivateWhisperMode();

    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void TriggerHorrorEffect(float Intensity = 0.8f, float Duration = 3.0f);

    // Settings Management
    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void RefreshInputBindings();

    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    bool IsListeningForKeyBind() const { return bIsListeningForKeyBind; }

    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void StartKeyBindListening();

    UFUNCTION(BlueprintCallable, Category = "Voice Input")
    void StopKeyBindListening();

protected:
    // Input event handlers
    void OnPushToTalkPressed();
    void OnPushToTalkReleased();
    void OnMuteTogglePressed();
    void OnVoiceTypeCyclePressed();
    void OnVoiceVolumeUpPressed();
    void OnVoiceVolumeDownPressed();
    void OnWhisperModeTogglePressed();

    // Key binding capture
    void OnAnyKeyPressed(FKey Key);
    bool HandleKeyInput(FKey Key, EInputEvent InputEvent);

private:
    // Component references
    UPROPERTY()
    UVoiceChatManager* VoiceChatManager;

    UPROPERTY()
    UHorrorGameSettings* GameSettings;

    UPROPERTY()
    class UInputComponent* BoundInputComponent;

    // Internal state
    bool bIsPushToTalkPressed;
    bool bIsWhisperModeActive;
    bool bIsListeningForKeyBind;

    // Horror effects
    FTimerHandle HorrorEffectTimer;
    float CurrentHorrorIntensity;

    // Input binding names - these should match your project's input settings
    static const FName PushToTalkActionName;
    static const FName MuteToggleActionName;
    static const FName VoiceTypeCycleActionName;
    static const FName VoiceVolumeUpActionName;
    static const FName VoiceVolumeDownActionName;
    static const FName WhisperModeToggleActionName;

    // Settings event handlers
    UFUNCTION()
    void OnGameSettingsPushToTalkKeyChanged(FKey NewKey);

    UFUNCTION()
    void OnGameSettingsVoiceTypeChanged(int32 NewType);

    // Helper functions
    void UpdatePushToTalkBinding();
    void ApplyHorrorEffectToVoice(float Intensity);
    void ClearHorrorEffect();
};