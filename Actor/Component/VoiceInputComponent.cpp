#include "VoiceInputComponent.h"
#include "HorrorGame/GameInstance/VoiceChatManager.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "TimerManager.h"

// Define input action names
const FName UVoiceInputComponent::PushToTalkActionName = TEXT("PushToTalk");
const FName UVoiceInputComponent::MuteToggleActionName = TEXT("MuteToggle");
const FName UVoiceInputComponent::VoiceTypeCycleActionName = TEXT("VoiceTypeCycle");
const FName UVoiceInputComponent::VoiceVolumeUpActionName = TEXT("VoiceVolumeUp");
const FName UVoiceInputComponent::VoiceVolumeDownActionName = TEXT("VoiceVolumeDown");
const FName UVoiceInputComponent::WhisperModeToggleActionName = TEXT("WhisperModeToggle");

UVoiceInputComponent::UVoiceInputComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    bIsPushToTalkPressed = false;
    bIsWhisperModeActive = false;
    bIsListeningForKeyBind = false;
    CurrentHorrorIntensity = 0.0f;

    VoiceChatManager = nullptr;
    GameSettings = nullptr;
    BoundInputComponent = nullptr;
}

void UVoiceInputComponent::BeginPlay()
{
    Super::BeginPlay();

    // Get subsystems
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            VoiceChatManager = GameInstance->GetSubsystem<UVoiceChatManager>();
        }
    }

    GameSettings = UHorrorGameSettings::Get();

    // Bind to settings changes
    if (GameSettings)
    {
        GameSettings->OnPushToTalkKeyChanged.AddDynamic(this, &UVoiceInputComponent::OnGameSettingsPushToTalkKeyChanged);
        GameSettings->OnVoiceTypeChanged.AddDynamic(this, &UVoiceInputComponent::OnGameSettingsVoiceTypeChanged);
    }

    // Setup input bindings if we have a player controller
    if (AActor* Owner = GetOwner())
    {
        if (APlayerController* PC = Cast<APlayerController>(Owner))
        {
            if (UInputComponent* InputComp = PC->InputComponent)
            {
                SetupInputBindings(InputComp);
            }
        }
    }
}

void UVoiceInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(HorrorEffectTimer);
    }

    // Unbind from settings
    if (GameSettings)
    {
        GameSettings->OnPushToTalkKeyChanged.RemoveDynamic(this, &UVoiceInputComponent::OnGameSettingsPushToTalkKeyChanged);
        GameSettings->OnVoiceTypeChanged.RemoveDynamic(this, &UVoiceInputComponent::OnGameSettingsVoiceTypeChanged);
    }

    // Remove input bindings
    if (BoundInputComponent)
    {
        RemoveInputBindings(BoundInputComponent);
    }

    Super::EndPlay(EndPlayReason);
}

void UVoiceInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update horror effects
    if (CurrentHorrorIntensity > 0.0f)
    {
        CurrentHorrorIntensity = FMath::Max(0.0f, CurrentHorrorIntensity - DeltaTime * 0.3f);

        if (VoiceChatManager)
        {
            VoiceChatManager->ApplyFearDistortion(CurrentHorrorIntensity);
        }
    }

    // Handle key binding listening
    if (bIsListeningForKeyBind)
    {
        // This would be handled by Slate's key capture in a real implementation
        // For now, we'll use a timer-based approach
    }
}

void UVoiceInputComponent::SetupInputBindings(UInputComponent* InputComponent)
{
    if (!InputComponent || !GameSettings)
    {
        return;
    }

    BoundInputComponent = InputComponent;

    // Bind push to talk
    FKey PushToTalkKey = GameSettings->GetPushToTalkKey();
    if (PushToTalkKey.IsValid())
    {
        InputComponent->BindKey(PushToTalkKey, IE_Pressed, this, &UVoiceInputComponent::OnPushToTalkPressed);
        InputComponent->BindKey(PushToTalkKey, IE_Released, this, &UVoiceInputComponent::OnPushToTalkReleased);
    }

    // Bind other voice controls (using default keys, should be configurable)
    InputComponent->BindKey(EKeys::M, IE_Pressed, this, &UVoiceInputComponent::OnMuteTogglePressed);
    InputComponent->BindKey(EKeys::N, IE_Pressed, this, &UVoiceInputComponent::OnVoiceTypeCyclePressed);
    InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &UVoiceInputComponent::OnVoiceVolumeUpPressed);
    InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &UVoiceInputComponent::OnVoiceVolumeDownPressed);
    InputComponent->BindKey(EKeys::B, IE_Pressed, this, &UVoiceInputComponent::OnWhisperModeTogglePressed);

    UE_LOG(LogTemp, Log, TEXT("Voice input bindings setup"));
}

void UVoiceInputComponent::RemoveInputBindings(UInputComponent* InputComponent)
{
    if (!InputComponent)
    {
        return;
    }

    // Note: UE4/5 doesn't have a direct way to remove specific bindings
    // In a real implementation, you might need to track and manually remove bindings
    // or rebuild the entire input component

    BoundInputComponent = nullptr;

    UE_LOG(LogTemp, Log, TEXT("Voice input bindings removed"));
}

void UVoiceInputComponent::StartPushToTalk()
{
    if (!VoiceChatManager || !GameSettings)
    {
        return;
    }

    // Only work in push-to-talk mode
    if (GameSettings->GetVoiceType() != 0)
    {
        return;
    }

    bIsPushToTalkPressed = true;
    VoiceChatManager->StartPushToTalk();

    UE_LOG(LogTemp, Log, TEXT("Push to talk started"));
}

void UVoiceInputComponent::StopPushToTalk()
{
    if (!VoiceChatManager)
    {
        return;
    }

    bIsPushToTalkPressed = false;
    VoiceChatManager->StopPushToTalk();

    UE_LOG(LogTemp, Log, TEXT("Push to talk stopped"));
}

void UVoiceInputComponent::ToggleMute()
{
    if (!VoiceChatManager)
    {
        return;
    }

    bool bCurrentlyMuted = VoiceChatManager->IsMicrophoneMuted();
    VoiceChatManager->SetMicrophoneMuted(!bCurrentlyMuted);

    UE_LOG(LogTemp, Log, TEXT("Mute toggled: %s"), !bCurrentlyMuted ? TEXT("Muted") : TEXT("Unmuted"));
}

void UVoiceInputComponent::CycleVoiceType()
{
    if (!GameSettings)
    {
        return;
    }

    int32 CurrentType = GameSettings->GetVoiceType();
    int32 NextType = (CurrentType + 1) % 3; // Cycle through 0, 1, 2

    GameSettings->SetVoiceType(NextType);

    FString TypeNames[] = { TEXT("Push to Talk"), TEXT("Open Mic"), TEXT("Voice Activation") };
    UE_LOG(LogTemp, Log, TEXT("Voice type changed to: %s"), *TypeNames[NextType]);
}

void UVoiceInputComponent::IncreaseVoiceVolume(float Amount)
{
    if (!GameSettings)
    {
        return;
    }

    float CurrentVolume = GameSettings->GetVoiceVolume();
    float NewVolume = FMath::Clamp(CurrentVolume + Amount, 0.0f, 1.0f);

    GameSettings->SetVoiceVolume(NewVolume);

    UE_LOG(LogTemp, Log, TEXT("Voice volume increased to: %f"), NewVolume);
}

void UVoiceInputComponent::DecreaseVoiceVolume(float Amount)
{
    if (!GameSettings)
    {
        return;
    }

    float CurrentVolume = GameSettings->GetVoiceVolume();
    float NewVolume = FMath::Clamp(CurrentVolume - Amount, 0.0f, 1.0f);

    GameSettings->SetVoiceVolume(NewVolume);

    UE_LOG(LogTemp, Log, TEXT("Voice volume decreased to: %f"), NewVolume);
}

void UVoiceInputComponent::ActivateWhisperMode()
{
    if (!VoiceChatManager || bIsWhisperModeActive)
    {
        return;
    }

    bIsWhisperModeActive = true;
    VoiceChatManager->EnableWhispering(true);

    UE_LOG(LogTemp, Log, TEXT("Whisper mode activated"));
}

void UVoiceInputComponent::DeactivateWhisperMode()
{
    if (!VoiceChatManager || !bIsWhisperModeActive)
    {
        return;
    }

    bIsWhisperModeActive = false;
    VoiceChatManager->EnableWhispering(false);

    UE_LOG(LogTemp, Log, TEXT("Whisper mode deactivated"));
}

void UVoiceInputComponent::TriggerHorrorEffect(float Intensity, float Duration)
{
    if (!VoiceChatManager)
    {
        return;
    }

    CurrentHorrorIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    // Apply immediate horror effects
    VoiceChatManager->ApplyFearDistortion(CurrentHorrorIntensity);
    VoiceChatManager->EnableStaticNoise(true, CurrentHorrorIntensity * 0.5f);

    // Set timer to clear effects
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(HorrorEffectTimer, [this]()
            {
                ClearHorrorEffect();
            }, Duration, false);
    }

    UE_LOG(LogTemp, Log, TEXT("Horror voice effect triggered: Intensity=%f, Duration=%f"), Intensity, Duration);
}

void UVoiceInputComponent::RefreshInputBindings()
{
    if (BoundInputComponent)
    {
        RemoveInputBindings(BoundInputComponent);
        SetupInputBindings(BoundInputComponent);
    }
}

void UVoiceInputComponent::StartKeyBindListening()
{
    if (bIsListeningForKeyBind)
    {
        return;
    }

    bIsListeningForKeyBind = true;

    // In a real implementation, you would capture input from Slate
    // For now, we'll simulate this with a timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(HorrorEffectTimer, [this]()
            {
                // Simulate key capture - in reality, this would be handled by input events
                StopKeyBindListening();

                // Example: set a new key
                if (GameSettings)
                {
                    GameSettings->SetPushToTalkKey(EKeys::T);
                }

            }, 3.0f, false);
    }

    UE_LOG(LogTemp, Log, TEXT("Started listening for key bind"));
}

void UVoiceInputComponent::StopKeyBindListening()
{
    if (!bIsListeningForKeyBind)
    {
        return;
    }

    bIsListeningForKeyBind = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(HorrorEffectTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("Stopped listening for key bind"));
}

// Input Event Handlers
void UVoiceInputComponent::OnPushToTalkPressed()
{
    StartPushToTalk();
}

void UVoiceInputComponent::OnPushToTalkReleased()
{
    StopPushToTalk();
}

void UVoiceInputComponent::OnMuteTogglePressed()
{
    ToggleMute();
}

void UVoiceInputComponent::OnVoiceTypeCyclePressed()
{
    CycleVoiceType();
}

void UVoiceInputComponent::OnVoiceVolumeUpPressed()
{
    IncreaseVoiceVolume(0.1f);
}

void UVoiceInputComponent::OnVoiceVolumeDownPressed()
{
    DecreaseVoiceVolume(0.1f);
}

void UVoiceInputComponent::OnWhisperModeTogglePressed()
{
    if (bIsWhisperModeActive)
    {
        DeactivateWhisperMode();
    }
    else
    {
        ActivateWhisperMode();
    }
}

// Settings Event Handlers
void UVoiceInputComponent::OnGameSettingsPushToTalkKeyChanged(FKey NewKey)
{
    UE_LOG(LogTemp, Log, TEXT("Push to talk key changed to: %s"), *NewKey.ToString());

    // Refresh input bindings to use the new key
    RefreshInputBindings();
}

void UVoiceInputComponent::OnGameSettingsVoiceTypeChanged(int32 NewType)
{
    UE_LOG(LogTemp, Log, TEXT("Voice type changed to: %d"), NewType);

    // Stop push to talk if we're switching away from that mode
    if (NewType != 0 && bIsPushToTalkPressed)
    {
        StopPushToTalk();
    }
}

// Helper Functions
void UVoiceInputComponent::UpdatePushToTalkBinding()
{
    // Refresh the push to talk binding with the current key from settings
    RefreshInputBindings();
}

void UVoiceInputComponent::ApplyHorrorEffectToVoice(float Intensity)
{
    if (!VoiceChatManager)
    {
        return;
    }

    CurrentHorrorIntensity = Intensity;
    VoiceChatManager->ApplyFearDistortion(Intensity);
    VoiceChatManager->EnableStaticNoise(Intensity > 0.1f, Intensity * 0.3f);
}

void UVoiceInputComponent::ClearHorrorEffect()
{
    if (!VoiceChatManager)
    {
        return;
    }

    CurrentHorrorIntensity = 0.0f;
    VoiceChatManager->ApplyFearDistortion(0.0f);
    VoiceChatManager->EnableStaticNoise(false);

    UE_LOG(LogTemp, Log, TEXT("Horror voice effects cleared"));
}