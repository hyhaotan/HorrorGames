#include "VoiceChatManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"

UVoiceChatManager::UVoiceChatManager()
{
    bIsConnected = false;
    bIsRecording = false;
    bIsMuted = false;
    bIsPushToTalkActive = false;
    bProximityVoiceEnabled = false;
    bHorrorEffectsEnabled = false;
    bVoiceCutoutActive = false;

    InputVolume = 0.8f;
    OutputVolume = 0.8f;
    ProximityRange = 2000.0f; // 20 meters in cm
    CurrentVoiceLevel = 0.0f;
    VoiceThreshold = 0.3f;
    HorrorDistortionIntensity = 0.0f;
    StaticNoiseIntensity = 0.0f;

    CurrentChannelName = TEXT("");
}

void UVoiceChatManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    GameSettings = UHorrorGameSettings::Get();
    if (GameSettings)
    {
        // Bind to settings changes
        GameSettings->OnSettingsSimpleChanged.AddDynamic(this, &UVoiceChatManager::OnGameSettingsChanged);
    }

    UE_LOG(LogTemp, Log, TEXT("VoiceChatManager initialized"));
}

void UVoiceChatManager::Deinitialize()
{
    ShutdownVoiceChat();

    if (GameSettings)
    {
        GameSettings->OnSettingsSimpleChanged.RemoveDynamic(this, &UVoiceChatManager::OnGameSettingsChanged);
    }

    Super::Deinitialize();
}

bool UVoiceChatManager::InitializeVoiceChat()
{
    if (bIsConnected)
    {
        return true;
    }

    // Initialize voice chat SDK here
    // This would typically involve initializing a third-party voice chat service
    // like Discord GameSDK, Vivox, or Agora

    UE_LOG(LogTemp, Log, TEXT("Initializing voice chat..."));

    // Simulate initialization
    bIsConnected = true;

    // Apply current settings
    ApplySettingsFromGameSettings();

    // Start update timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(VoiceUpdateTimer, [this]()
            {
                UpdateVoiceActivity(0.1f);
            }, 0.1f, true);

        World->GetTimerManager().SetTimer(ProximityUpdateTimer, [this]()
            {
                UpdateProximityVoice();
            }, 0.2f, true);
    }

    OnVoiceChatStatusChanged.Broadcast(bIsConnected);

    UE_LOG(LogTemp, Log, TEXT("Voice chat initialized successfully"));
    return true;
}

void UVoiceChatManager::ShutdownVoiceChat()
{
    if (!bIsConnected)
    {
        return;
    }

    // Stop recording
    StopRecording();

    // Leave current channel
    LeaveVoiceChannel();

    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(VoiceUpdateTimer);
        World->GetTimerManager().ClearTimer(ProximityUpdateTimer);
        World->GetTimerManager().ClearTimer(VoiceCutoutTimer);
    }

    // Shutdown voice chat SDK
    bIsConnected = false;

    OnVoiceChatStatusChanged.Broadcast(bIsConnected);

    UE_LOG(LogTemp, Log, TEXT("Voice chat shutdown"));
}

bool UVoiceChatManager::JoinVoiceChannel(const FString& ChannelName)
{
    if (!bIsConnected || ChannelName.IsEmpty())
    {
        return false;
    }

    // Leave current channel if already in one
    if (!CurrentChannelName.IsEmpty())
    {
        LeaveVoiceChannel();
    }

    CurrentChannelName = ChannelName;

    // Join the channel via voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Joined voice channel: %s"), *ChannelName);

    return true;
}

void UVoiceChatManager::LeaveVoiceChannel()
{
    if (CurrentChannelName.IsEmpty())
    {
        return;
    }

    // Leave the channel via voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Left voice channel: %s"), *CurrentChannelName);

    CurrentChannelName = TEXT("");
    ConnectedPlayers.Empty();
}

TArray<FVoiceChatPlayer> UVoiceChatManager::GetVoiceChatPlayers() const
{
    return ConnectedPlayers;
}

bool UVoiceChatManager::IsPlayerTalking(const FString& PlayerId) const
{
    for (const FVoiceChatPlayer& Player : ConnectedPlayers)
    {
        if (Player.PlayerId == PlayerId)
        {
            return Player.bIsTalking;
        }
    }
    return false;
}

void UVoiceChatManager::StartRecording()
{
    if (!bIsConnected || bIsRecording || bIsMuted)
    {
        return;
    }

    bIsRecording = true;

    UE_LOG(LogTemp, Log, TEXT("Started voice recording"));

    // Start microphone recording via voice chat SDK
}

void UVoiceChatManager::StopRecording()
{
    if (!bIsRecording)
    {
        return;
    }

    bIsRecording = false;

    UE_LOG(LogTemp, Log, TEXT("Stopped voice recording"));

    // Stop microphone recording via voice chat SDK
}

void UVoiceChatManager::SetMicrophoneMuted(bool bMuted)
{
    if (bIsMuted == bMuted)
    {
        return;
    }

    bIsMuted = bMuted;

    if (bMuted)
    {
        StopRecording();
    }

    UE_LOG(LogTemp, Log, TEXT("Microphone %s"), bMuted ? TEXT("muted") : TEXT("unmuted"));
}

void UVoiceChatManager::StartPushToTalk()
{
    if (!bIsConnected || bIsMuted)
    {
        return;
    }

    bIsPushToTalkActive = true;
    StartRecording();

    UE_LOG(LogTemp, Log, TEXT("Push to talk activated"));
}

void UVoiceChatManager::StopPushToTalk()
{
    if (!bIsPushToTalkActive)
    {
        return;
    }

    bIsPushToTalkActive = false;
    StopRecording();

    UE_LOG(LogTemp, Log, TEXT("Push to talk deactivated"));
}

void UVoiceChatManager::SetInputVolume(float Volume)
{
    InputVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

    // Apply input volume to voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Input volume set to: %f"), InputVolume);
}

void UVoiceChatManager::SetOutputVolume(float Volume)
{
    OutputVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

    // Apply output volume to voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Output volume set to: %f"), OutputVolume);
}

void UVoiceChatManager::MutePlayer(const FString& PlayerId, bool bMute)
{
    if (PlayerId.IsEmpty())
    {
        return;
    }

    MutedPlayers.Add(PlayerId, bMute);

    // Mute player in voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Player %s %s"), *PlayerId, bMute ? TEXT("muted") : TEXT("unmuted"));
}

bool UVoiceChatManager::IsPlayerMuted(const FString& PlayerId) const
{
    const bool* bMuted = MutedPlayers.Find(PlayerId);
    return bMuted ? *bMuted : false;
}

void UVoiceChatManager::SetPlayerVolume(const FString& PlayerId, float Volume)
{
    if (PlayerId.IsEmpty())
    {
        return;
    }

    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    PlayerVolumes.Add(PlayerId, Volume);

    // Set player volume in voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Player %s volume set to: %f"), *PlayerId, Volume);
}

float UVoiceChatManager::GetPlayerVolume(const FString& PlayerId) const
{
    const float* Volume = PlayerVolumes.Find(PlayerId);
    return Volume ? *Volume : 1.0f;
}

void UVoiceChatManager::EnableProximityVoice(bool bEnable)
{
    if (bProximityVoiceEnabled == bEnable)
    {
        return;
    }

    bProximityVoiceEnabled = bEnable;

    UE_LOG(LogTemp, Log, TEXT("Proximity voice %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UVoiceChatManager::SetProximityRange(float Range)
{
    ProximityRange = FMath::Clamp(Range, 100.0f, 5000.0f); // 1m to 50m

    UE_LOG(LogTemp, Log, TEXT("Proximity range set to: %f cm"), ProximityRange);
}

void UVoiceChatManager::EnableVoiceDistortion(bool bEnable, float Intensity)
{
    if (bEnable)
    {
        HorrorDistortionIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    }
    else
    {
        HorrorDistortionIntensity = 0.0f;
    }

    // Apply distortion effect to voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Voice distortion %s with intensity: %f"),
        bEnable ? TEXT("enabled") : TEXT("disabled"), HorrorDistortionIntensity);
}

void UVoiceChatManager::EnableVoiceEcho(bool bEnable, float Strength)
{
    // Apply echo effect to voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Voice echo %s with strength: %f"),
        bEnable ? TEXT("enabled") : TEXT("disabled"), Strength);
}

void UVoiceChatManager::EnableWhisperMode(bool bEnable)
{
    // Apply whisper effect to voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Whisper mode %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UVoiceChatManager::EnableNoiseGate(bool bEnable, float Threshold)
{
    VoiceThreshold = bEnable ? FMath::Clamp(Threshold, 0.0f, 1.0f) : 0.0f;

    // Apply noise gate to voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Noise gate %s with threshold: %f"),
        bEnable ? TEXT("enabled") : TEXT("disabled"), VoiceThreshold);
}

void UVoiceChatManager::SetVoiceThreshold(float Threshold)
{
    VoiceThreshold = FMath::Clamp(Threshold, 0.0f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Voice threshold set to: %f"), VoiceThreshold);
}

void UVoiceChatManager::ApplySettingsFromGameSettings()
{
    if (!GameSettings)
    {
        return;
    }

    // Apply basic voice settings
    SetMicrophoneMuted(GameSettings->GetVoiceMuted());
    SetInputVolume(GameSettings->GetVoiceVolume());
    SetVoiceThreshold(GameSettings->GetVoiceThreshold());

    // Apply proximity voice settings
    EnableProximityVoice(GameSettings->GetProximityVoice());
    SetProximityRange(GameSettings->ProximityVoiceRange * 100.0f); // Convert to cm

    // Apply voice effects
    EnableVoiceDistortion(GameSettings->GetVoiceDistortion(),
        GameSettings->VoiceDistortionIntensity);
    EnableVoiceEcho(GameSettings->bVoiceEcho, GameSettings->VoiceEchoStrength);
    EnableWhisperMode(GameSettings->GetWhisperMode());
    EnableNoiseGate(GameSettings->bVoiceNoiseGate, GameSettings->NoiseGateThreshold);

    UE_LOG(LogTemp, Log, TEXT("Applied settings from game settings"));
}

void UVoiceChatManager::OnGameSettingsChanged()
{
    ApplySettingsFromGameSettings();
}

// Horror Game Specific Features
void UVoiceChatManager::EnableHorrorEffects(bool bEnable)
{
    bHorrorEffectsEnabled = bEnable;

    if (!bEnable)
    {
        // Disable all horror effects
        EnableVoiceDistortion(false);
        EnableStaticNoise(false);
        HorrorDistortionIntensity = 0.0f;
        StaticNoiseIntensity = 0.0f;
    }

    UE_LOG(LogTemp, Log, TEXT("Horror voice effects %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UVoiceChatManager::ApplyFearDistortion(float Intensity)
{
    if (!bHorrorEffectsEnabled)
    {
        return;
    }

    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    EnableVoiceDistortion(Intensity > 0.0f, Intensity);

    UE_LOG(LogTemp, Log, TEXT("Fear distortion applied with intensity: %f"), Intensity);
}

void UVoiceChatManager::EnableStaticNoise(bool bEnable, float Intensity)
{
    if (!bHorrorEffectsEnabled && bEnable)
    {
        return;
    }

    StaticNoiseIntensity = bEnable ? FMath::Clamp(Intensity, 0.0f, 1.0f) : 0.0f;

    // Apply static noise effect to voice chat SDK
    UE_LOG(LogTemp, Log, TEXT("Static noise %s with intensity: %f"),
        bEnable ? TEXT("enabled") : TEXT("disabled"), StaticNoiseIntensity);
}

void UVoiceChatManager::EnableWhispering(bool bEnable)
{
    if (!bHorrorEffectsEnabled && bEnable)
    {
        return;
    }

    EnableWhisperMode(bEnable);

    // Additional horror-specific whispering effects
    UE_LOG(LogTemp, Log, TEXT("Horror whispering %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UVoiceChatManager::TriggerVoiceCutout(float Duration)
{
    if (!bHorrorEffectsEnabled || bVoiceCutoutActive)
    {
        return;
    }

    bVoiceCutoutActive = true;

    // Temporarily disable voice transmission
    if (bIsRecording)
    {
        StopRecording();
    }

    // Set timer to re-enable voice
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(VoiceCutoutTimer, [this]()
            {
                bVoiceCutoutActive = false;

                // Re-enable voice if it should be active
                if (GameSettings && !GameSettings->GetVoiceMuted())
                {
                    if (GameSettings->GetVoiceType() == 1) // Open mic
                    {
                        StartRecording();
                    }
                }

            }, Duration, false);
    }

    UE_LOG(LogTemp, Log, TEXT("Voice cutout triggered for %f seconds"), Duration);
}

// Private helper functions
void UVoiceChatManager::UpdateVoiceActivity(float DeltaTime)
{
    if (!bIsConnected || !bIsRecording)
    {
        CurrentVoiceLevel = FMath::FInterpTo(CurrentVoiceLevel, 0.0f, DeltaTime, 5.0f);
        return;
    }

    // Simulate voice activity level (in real implementation, this would come from microphone input)
    float TargetLevel = 0.0f;

    if (bIsPushToTalkActive || (GameSettings && GameSettings->GetVoiceType() == 1)) // Open mic
    {
        // Simulate some voice activity
        TargetLevel = FMath::RandRange(0.3f, 0.8f);
    }

    CurrentVoiceLevel = FMath::FInterpTo(CurrentVoiceLevel, TargetLevel, DeltaTime, 10.0f);

    // Check if voice level crosses threshold for voice activation
    if (GameSettings && GameSettings->GetVoiceType() == 2) // Voice activation
    {
        bool bShouldBeRecording = CurrentVoiceLevel > GameSettings->GetVoiceThreshold();

        if (bShouldBeRecording && !bIsRecording)
        {
            StartRecording();
        }
        else if (!bShouldBeRecording && bIsRecording && !bIsPushToTalkActive)
        {
            StopRecording();
        }
    }

    // Process voice effects
    ProcessVoiceEffects();
}

void UVoiceChatManager::UpdateProximityVoice()
{
    if (!bProximityVoiceEnabled || !bIsConnected)
    {
        return;
    }

    // Get local player
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* LocalPC = World->GetFirstPlayerController();
    if (!LocalPC || !LocalPC->GetPawn())
    {
        return;
    }

    FVector LocalPlayerLocation = LocalPC->GetPawn()->GetActorLocation();

    // Update distance for all connected players
    for (FVoiceChatPlayer& Player : ConnectedPlayers)
    {
        // In a real implementation, you would get other players' locations
        // For now, we'll simulate some distances
        float SimulatedDistance = FMath::RandRange(500.0f, 3000.0f);
        Player.DistanceFromLocalPlayer = SimulatedDistance;

        // Calculate volume based on distance
        float VolumeMultiplier = 1.0f;
        if (SimulatedDistance > ProximityRange)
        {
            VolumeMultiplier = 0.0f; // Out of range
        }
        else
        {
            VolumeMultiplier = 1.0f - (SimulatedDistance / ProximityRange);
        }

        // Apply proximity volume
        SetPlayerVolume(Player.PlayerId, VolumeMultiplier);
    }
}

void UVoiceChatManager::ProcessVoiceEffects()
{
    if (!bHorrorEffectsEnabled || !bIsRecording)
    {
        return;
    }

    // Process horror-specific voice effects
    if (HorrorDistortionIntensity > 0.0f)
    {
        // Apply real-time voice distortion
        // This would modify the audio stream in real implementation
    }

    if (StaticNoiseIntensity > 0.0f)
    {
        // Apply static noise overlay
        // This would add noise to the audio stream in real implementation
    }
}

void UVoiceChatManager::HandlePushToTalkInput()
{
    if (!GameSettings || GameSettings->GetVoiceType() != 0) // Not push to talk
    {
        return;
    }

    // In a real implementation, you would check if the push-to-talk key is pressed
    // This would be handled by the input system and call StartPushToTalk/StopPushToTalk
}