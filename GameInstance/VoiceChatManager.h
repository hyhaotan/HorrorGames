#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "VoiceChatManager.generated.h"

USTRUCT(BlueprintType)
struct FVoiceChatPlayer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly)
    FString PlayerId;

    UPROPERTY(BlueprintReadOnly)
    bool bIsMuted;

    UPROPERTY(BlueprintReadOnly)
    bool bIsTalking;

    UPROPERTY(BlueprintReadOnly)
    float VoiceLevel;

    UPROPERTY(BlueprintReadOnly)
    float DistanceFromLocalPlayer;

    FVoiceChatPlayer()
    {
        PlayerName = TEXT("");
        PlayerId = TEXT("");
        bIsMuted = false;
        bIsTalking = false;
        VoiceLevel = 0.0f;
        DistanceFromLocalPlayer = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStartedTalking, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStoppedTalking, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoinedVoiceChat, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeftVoiceChat, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceChatStatusChanged, bool, bIsConnected);

UCLASS()
class HORRORGAME_API UVoiceChatManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UVoiceChatManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Voice Chat Connection
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool InitializeVoiceChat();

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void ShutdownVoiceChat();

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsVoiceChatConnected() const { return bIsConnected; }

    // Player Management
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool JoinVoiceChannel(const FString& ChannelName);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void LeaveVoiceChannel();

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    TArray<FVoiceChatPlayer> GetVoiceChatPlayers() const;

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsPlayerTalking(const FString& PlayerId) const;

    // Microphone Control
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void StartRecording();

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void StopRecording();

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsRecording() const { return bIsRecording; }

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetMicrophoneMuted(bool bMuted);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsMicrophoneMuted() const { return bIsMuted; }

    // Push to Talk
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void StartPushToTalk();

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void StopPushToTalk();

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsPushToTalkActive() const { return bIsPushToTalkActive; }

    // Volume Control
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetInputVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    float GetInputVolume() const { return InputVolume; }

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetOutputVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    float GetOutputVolume() const { return OutputVolume; }

    // Player-specific Controls
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void MutePlayer(const FString& PlayerId, bool bMute);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsPlayerMuted(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetPlayerVolume(const FString& PlayerId, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    float GetPlayerVolume(const FString& PlayerId) const;

    // Proximity Voice
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void EnableProximityVoice(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    bool IsProximityVoiceEnabled() const { return bProximityVoiceEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetProximityRange(float Range);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    float GetProximityRange() const { return ProximityRange; }

    // Voice Effects
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void EnableVoiceDistortion(bool bEnable, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void EnableVoiceEcho(bool bEnable, float Strength = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void EnableWhisperMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void EnableNoiseGate(bool bEnable, float Threshold = 0.1f);

    // Audio Analysis
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    float GetCurrentVoiceLevel() const { return CurrentVoiceLevel; }

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    float GetVoiceThreshold() const { return VoiceThreshold; }

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void SetVoiceThreshold(float Threshold);

    // Settings Integration
    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void ApplySettingsFromGameSettings();

    UFUNCTION(BlueprintCallable, Category = "Voice Chat")
    void OnGameSettingsChanged();

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnPlayerStartedTalking OnPlayerStartedTalking;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerStoppedTalking OnPlayerStoppedTalking;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerJoinedVoiceChat OnPlayerJoinedVoiceChat;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerLeftVoiceChat OnPlayerLeftVoiceChat;

    UPROPERTY(BlueprintAssignable)
    FOnVoiceChatStatusChanged OnVoiceChatStatusChanged;

    // Horror Game Specific Features
    UFUNCTION(BlueprintCallable, Category = "Horror Voice")
    void EnableHorrorEffects(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Horror Voice")
    void ApplyFearDistortion(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Horror Voice")
    void EnableStaticNoise(bool bEnable, float Intensity = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Horror Voice")
    void EnableWhispering(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Horror Voice")
    void TriggerVoiceCutout(float Duration = 2.0f);

protected:
    // Internal state
    UPROPERTY()
    bool bIsConnected;

    UPROPERTY()
    bool bIsRecording;

    UPROPERTY()
    bool bIsMuted;

    UPROPERTY()
    bool bIsPushToTalkActive;

    UPROPERTY()
    bool bProximityVoiceEnabled;

    UPROPERTY()
    bool bHorrorEffectsEnabled;

    UPROPERTY()
    float InputVolume;

    UPROPERTY()
    float OutputVolume;

    UPROPERTY()
    float ProximityRange;

    UPROPERTY()
    float CurrentVoiceLevel;

    UPROPERTY()
    float VoiceThreshold;

    UPROPERTY()
    FString CurrentChannelName;

    UPROPERTY()
    TArray<FVoiceChatPlayer> ConnectedPlayers;

    UPROPERTY()
    TMap<FString, bool> MutedPlayers;

    UPROPERTY()
    TMap<FString, float> PlayerVolumes;

    UPROPERTY()
    UHorrorGameSettings* GameSettings;

private:
    // Internal functions
    void UpdateVoiceActivity(float DeltaTime);
    void UpdateProximityVoice();
    void ProcessVoiceEffects();
    void HandlePushToTalkInput();

    // Timer handles
    FTimerHandle VoiceUpdateTimer;
    FTimerHandle ProximityUpdateTimer;
    FTimerHandle VoiceCutoutTimer;

    // Internal state
    bool bVoiceCutoutActive;
    float HorrorDistortionIntensity;
    float StaticNoiseIntensity;
};