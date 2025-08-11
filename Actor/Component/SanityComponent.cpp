#include "SanityComponent.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Component/HeadbobComponent.h"
#include "HorrorGame/Widget/Progress/SanityWidget.h"
#include "Components/PostProcessComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

USanityComponent::USanityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize state
    CurrentSanity = StartingSanity;
    LastUpdateTime = 0.0f;
    bEnvironmentalDrainActive = false;
    bInSafeZone = false;
    bSystemPaused = false;
    bInPanicAttack = false;
    CurrentState = ESanityState::Stable;
    PreviousState = ESanityState::Stable;

    LastPanicTime = -999.0f;
    LastHallucinationTime = -999.0f;

    // Initialize threshold tracking
    ThresholdTriggered.SetNum(SanityThresholds.Num());
    for (int32 i = 0; i < ThresholdTriggered.Num(); i++)
    {
        ThresholdTriggered[i] = false;
    }

    // Create timeline component
    SanityTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SanityTimeline"));

    // Create audio component for heartbeat
    HeartbeatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("HeartbeatAudio"));
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->bAutoActivate = false;
        HeartbeatAudioComponent->SetVolumeMultiplier(0.0f);
    }
}

void USanityComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
        HeadbobComponent = OwnerCharacter->FindComponentByClass<UHeadbobComponent>();
        PostProcessComponent = OwnerCharacter->FindComponentByClass<UPostProcessComponent>();

        if (HeartbeatAudioComponent)
        {
            HeartbeatAudioComponent->AttachToComponent(
                OwnerCharacter->GetRootComponent(),
                FAttachmentTransformRules::KeepRelativeTransform
            );
        }
    }

    if (!ValidateComponents())
    {
        UE_LOG(LogTemp, Warning, TEXT("SanityComponent: Some components missing"));
    }

    // Initialize systems
    CurrentSanity = StartingSanity;
    InitializeSanityTimeline();
    SetupSanityWidget();
    CheckSanityState();

    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void USanityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bSystemPaused) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Throttle updates for performance
    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        LastUpdateTime = CurrentTime;

        CheckSanityThresholds();
        CheckSanityState();
        CheckRandomEvents();
        UpdatePostProcessEffects();
        UpdateAudioEffects();
        NotifyHeadbobComponent();

        // Natural recovery in safe zones
        if (bInSafeZone && GetSanityPercent() < 1.0f)
        {
            RecoverSanity(SafeZoneRecoveryRate * UpdateFrequency, false);
        }
        // Very slow natural recovery when not in panic
        else if (!bInPanicAttack && GetSanityPercent() > 0.8f)
        {
            RecoverSanity(NaturalRecoveryRate * UpdateFrequency, false);
        }
    }
}

void USanityComponent::RecoverSanity(float Delta, bool bInstant)
{
    if (Delta <= 0.0f) return;
    ProcessSanityChange(Delta, bInstant);
}

void USanityComponent::DrainSanity(float Delta, bool bInstant)
{
    if (Delta <= 0.0f) return;
    ProcessSanityChange(-Delta, bInstant);
}

void USanityComponent::ProcessSanityChange(float Delta, bool bInstant)
{
    float PreviousSanity = CurrentSanity;
    CurrentSanity = FMath::Clamp(CurrentSanity + Delta, 0.0f, MaxSanity);

    if (FMath::Abs(CurrentSanity - PreviousSanity) > 0.01f)
    {
        // Update UI
        if (SanityWidget)
        {
            SanityWidget->SetSanityPercent(GetSanityPercent());
        }

        // Broadcast change
        OnSanityChanged.Broadcast(GetSanityPercent());

        if (bDebugSanity)
        {
            FString ChangeType = Delta > 0 ? TEXT("recovered") : TEXT("lost");
            UE_LOG(LogTemp, Log, TEXT("Sanity %s: %.2f -> %.2f (%.1f%%)"),
                *ChangeType, PreviousSanity, CurrentSanity, GetSanityPercent() * 100.0f);
        }
    }
}

void USanityComponent::SetSanity(float NewSanity)
{
    float Delta = NewSanity - CurrentSanity;
    ProcessSanityChange(Delta, true);
}

ESanityState USanityComponent::GetSanityState() const
{
    float SanityPercent = GetSanityPercent();

    if (SanityPercent >= 0.8f) return ESanityState::Stable;
    else if (SanityPercent >= 0.6f) return ESanityState::Nervous;
    else if (SanityPercent >= 0.4f) return ESanityState::Anxious;
    else if (SanityPercent >= 0.2f) return ESanityState::Unstable;
    else return ESanityState::Critical;
}

void USanityComponent::StartEnvironmentalDrain()
{
    bEnvironmentalDrainActive = true;
    if (SanityTimeline && !SanityTimeline->IsPlaying())
    {
        SanityTimeline->PlayFromStart();
    }
}

void USanityComponent::StopEnvironmentalDrain()
{
    bEnvironmentalDrainActive = false;
    if (SanityTimeline && SanityTimeline->IsPlaying())
    {
        SanityTimeline->Stop();
    }
}

void USanityComponent::PauseSanitySystem()
{
    bSystemPaused = true;
    if (SanityTimeline && SanityTimeline->IsPlaying())
    {
        SanityTimeline->Stop();
    }
}

void USanityComponent::ResumeSanitySystem()
{
    bSystemPaused = false;
    if (bEnvironmentalDrainActive && SanityTimeline && !SanityTimeline->IsPlaying())
    {
        SanityTimeline->Play();
    }
}

void USanityComponent::OnMonsterSighting(float SanityLoss)
{
    DrainSanity(SanityLoss, true); // Instant drain for dramatic events

    // Trigger additional effects based on current state
    if (GetSanityState() <= ESanityState::Anxious)
    {
        if (PlayerController && LowSanityShake)
        {
            PlayerController->PlayerCameraManager->StartCameraShake(LowSanityShake, 0.8f);
        }
    }

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Log, TEXT("Monster sighting! Sanity loss: %.2f"), SanityLoss);
    }
}

void USanityComponent::OnMonsterProximity(float SanityLoss)
{
    DrainSanity(SanityLoss, false); // Gradual drain for proximity

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Log, TEXT("Monster proximity! Sanity loss: %.2f"), SanityLoss);
    }
}

void USanityComponent::OnSupernaturalEvent(float SanityLoss)
{
    DrainSanity(SanityLoss, true);

    // Always trigger camera shake for supernatural events
    if (PlayerController && LowSanityShake)
    {
        PlayerController->PlayerCameraManager->StartCameraShake(LowSanityShake, 1.0f);
    }

    // Play supernatural sound effect
    if (WhisperSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), WhisperSound, 0.7f);
    }

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Log, TEXT("Supernatural event! Sanity loss: %.2f"), SanityLoss);
    }
}

void USanityComponent::OnJumpScare(float SanityLoss)
{
    DrainSanity(SanityLoss, true);

    // Intense camera shake for jump scares
    if (PlayerController && PanicShake)
    {
        PlayerController->PlayerCameraManager->StartCameraShake(PanicShake, 1.0f);
    }

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Log, TEXT("Jump scare! Sanity loss: %.2f"), SanityLoss);
    }
}

void USanityComponent::OnCorpseDiscovery(float SanityLoss)
{
    DrainSanity(SanityLoss, true);

    // Corpse discovery is very traumatic - might trigger panic
    if (GetSanityState() <= ESanityState::Unstable && FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        TriggerPanicAttack();
    }
}

void USanityComponent::OnSeeGhost(float SanityLoss)
{
    DrainSanity(SanityLoss, true);
    // Ghost sightings are very traumatic - might trigger panic
    if (GetSanityState() <= ESanityState::Unstable && FMath::RandRange(0.0f, 1.0f) < 0.5f)
    {
        TriggerPanicAttack();
    }
}

void USanityComponent::OnDarknessExposure(float SanityLoss)
{
    // This should be called periodically while in darkness
    DrainSanity(SanityLoss, false);
}

void USanityComponent::OnSafeZoneEnter()
{
    bInSafeZone = true;

    // Immediately stop environmental drain in safe zones
    StopEnvironmentalDrain();

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Log, TEXT("Entered safe zone - sanity recovery enabled"));
    }
}

void USanityComponent::OnLightSource(float RecoveryAmount)
{
    RecoverSanity(RecoveryAmount, false);

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Log, TEXT("Light source found! Sanity recovery: %.2f"), RecoveryAmount);
    }
}

void USanityComponent::TriggerPanicAttack()
{
    if (!CanTriggerPanicAttack()) return;

    bInPanicAttack = true;
    LastPanicTime = GetWorld()->GetTimeSeconds();

    // Broadcast panic event
    OnPanicAttack.Broadcast();

    // Play panic sound
    if (PanicSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), PanicSound);
    }

    // Intense camera shake
    if (PlayerController && PanicShake)
    {
        PlayerController->PlayerCameraManager->StartCameraShake(PanicShake, 1.5f);
    }

    // End panic attack after duration
    FTimerHandle PanicTimer;
    GetWorld()->GetTimerManager().SetTimer(
        PanicTimer,
        [this]() {
            bInPanicAttack = false;
            if (bDebugSanity) UE_LOG(LogTemp, Log, TEXT("Panic attack ended"));
        },
        PanicAttackDuration,
        false
    );

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Warning, TEXT("PANIC ATTACK TRIGGERED!"));
    }
}

void USanityComponent::TriggerHallucination()
{
    if (!CanTriggerHallucination()) return;

    LastHallucinationTime = GetWorld()->GetTimeSeconds();

    // Broadcast hallucination event
    OnHallucination.Broadcast();

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Warning, TEXT("HALLUCINATION TRIGGERED!"));
    }
}

void USanityComponent::InitializeSanityTimeline()
{
    if (!SanityDrainCurve || !SanityTimeline) return;

    FOnTimelineFloat ProgressDelegate;
    ProgressDelegate.BindUFunction(this, TEXT("HandleEnvironmentalDrain"));
    SanityTimeline->AddInterpFloat(SanityDrainCurve, ProgressDelegate);
    SanityTimeline->SetLooping(true);
}

void USanityComponent::SetupSanityWidget()
{
    if (!SanityWidgetClass) return;

    SanityWidget = CreateWidget<USanityWidget>(GetWorld(), SanityWidgetClass);
    if (SanityWidget)
    {
        SanityWidget->AddToViewport();
        SanityWidget->SetSanityPercent(GetSanityPercent());
    }
}

void USanityComponent::UpdatePostProcessEffects()
{
    if (!PostProcessComponent) return;

    const float SanityPercent = GetSanityPercent();
    const float Intensity = GetPostProcessIntensity();

    if (SanityPercent <= LowSanityEffectThreshold)
    {
        PostProcessComponent->BlendWeight = Intensity;

        // Progressive effects based on sanity state
        switch (GetSanityState())
        {
        case ESanityState::Nervous:
            // Minimal effects
            PostProcessComponent->Settings.bOverride_VignetteIntensity = true;
            PostProcessComponent->Settings.VignetteIntensity = FMath::Lerp(0.0f, 0.3f, Intensity);
            break;

        case ESanityState::Anxious:
            // Add chromatic aberration and slight motion blur
            PostProcessComponent->Settings.bOverride_VignetteIntensity = true;
            PostProcessComponent->Settings.VignetteIntensity = FMath::Lerp(0.3f, 0.5f, Intensity);

            PostProcessComponent->Settings.bOverride_SceneFringeIntensity = true;
            PostProcessComponent->Settings.SceneFringeIntensity = FMath::Lerp(0.0f, 2.0f, Intensity);
            break;

        case ESanityState::Unstable:
            // Stronger effects
            PostProcessComponent->Settings.bOverride_VignetteIntensity = true;
            PostProcessComponent->Settings.VignetteIntensity = FMath::Lerp(0.5f, 0.7f, Intensity);

            PostProcessComponent->Settings.bOverride_SceneFringeIntensity = true;
            PostProcessComponent->Settings.SceneFringeIntensity = FMath::Lerp(2.0f, 4.0f, Intensity);

            PostProcessComponent->Settings.bOverride_MotionBlurAmount = true;
            PostProcessComponent->Settings.MotionBlurAmount = FMath::Lerp(0.0f, 0.5f, Intensity);
            break;

        case ESanityState::Critical:
            // Maximum effects
            PostProcessComponent->Settings.bOverride_VignetteIntensity = true;
            PostProcessComponent->Settings.VignetteIntensity = FMath::Lerp(0.7f, 1.0f, Intensity);

            PostProcessComponent->Settings.bOverride_SceneFringeIntensity = true;
            PostProcessComponent->Settings.SceneFringeIntensity = FMath::Lerp(4.0f, 6.0f, Intensity);

            PostProcessComponent->Settings.bOverride_MotionBlurAmount = true;
            PostProcessComponent->Settings.MotionBlurAmount = FMath::Lerp(0.5f, 1.0f, Intensity);

            PostProcessComponent->Settings.bOverride_BloomIntensity = true;
            PostProcessComponent->Settings.BloomIntensity = FMath::Lerp(1.0f, 2.0f, Intensity);

            PostProcessComponent->Settings.bOverride_DepthOfFieldFstop = true;
            PostProcessComponent->Settings.DepthOfFieldFstop = FMath::Lerp(22.0f, 1.4f, Intensity);
            break;

        default:
            break;
        }
    }
    else
    {
        // Disable post process effects when sanity is high
        PostProcessComponent->BlendWeight = 0.0f;
    }
}

void USanityComponent::UpdateAudioEffects()
{
    if (!HeartbeatAudioComponent || !HeartbeatSound) return;

    const float SanityPercent = GetSanityPercent();
    const ESanityState CurrentSanityState = GetSanityState();

    // Control heartbeat audio based on sanity level
    switch (CurrentSanityState)
    {
    case ESanityState::Stable:
    case ESanityState::Nervous:
        // No heartbeat or very quiet
        if (HeartbeatAudioComponent->IsPlaying())
        {
            HeartbeatAudioComponent->Stop();
        }
        break;

    case ESanityState::Anxious:
        // Light heartbeat
        if (!HeartbeatAudioComponent->IsPlaying())
        {
            HeartbeatAudioComponent->SetSound(HeartbeatSound);
            HeartbeatAudioComponent->Play();
        }
        HeartbeatAudioComponent->SetVolumeMultiplier(0.3f);
        HeartbeatAudioComponent->SetPitchMultiplier(1.0f);
        break;

    case ESanityState::Unstable:
        // Moderate heartbeat
        if (!HeartbeatAudioComponent->IsPlaying())
        {
            HeartbeatAudioComponent->SetSound(HeartbeatSound);
            HeartbeatAudioComponent->Play();
        }
        HeartbeatAudioComponent->SetVolumeMultiplier(0.6f);
        HeartbeatAudioComponent->SetPitchMultiplier(1.2f);
        break;

    case ESanityState::Critical:
        // Intense heartbeat
        if (!HeartbeatAudioComponent->IsPlaying())
        {
            HeartbeatAudioComponent->SetSound(HeartbeatSound);
            HeartbeatAudioComponent->Play();
        }
        HeartbeatAudioComponent->SetVolumeMultiplier(0.9f);
        HeartbeatAudioComponent->SetPitchMultiplier(1.4f);
        break;
    }

    // Extra intensity during panic attacks
    if (bInPanicAttack)
    {
        HeartbeatAudioComponent->SetVolumeMultiplier(1.0f);
        HeartbeatAudioComponent->SetPitchMultiplier(1.6f);
    }
}

void USanityComponent::CheckSanityThresholds()
{
    const float SanityPercent = GetSanityPercent();

    for (int32 i = 0; i < SanityThresholds.Num(); i++)
    {
        if (SanityPercent <= SanityThresholds[i] && !ThresholdTriggered[i])
        {
            ThresholdTriggered[i] = true;
            OnSanityThresholdReached.Broadcast(SanityThresholds[i]);

            if (bDebugSanity)
            {
                UE_LOG(LogTemp, Log, TEXT("Sanity threshold reached: %.1f%%"), SanityThresholds[i] * 100.0f);
            }
        }
        else if (SanityPercent > SanityThresholds[i] && ThresholdTriggered[i])
        {
            // Reset threshold if sanity recovers above it
            ThresholdTriggered[i] = false;
        }
    }
}

void USanityComponent::CheckSanityState()
{
    PreviousState = CurrentState;
    CurrentState = GetSanityState();

    // Notify when state changes
    if (CurrentState != PreviousState)
    {
        if (bDebugSanity)
        {
            UE_LOG(LogTemp, Warning, TEXT("Sanity state changed from %d to %d"),
                (int32)PreviousState, (int32)CurrentState);
        }

        // Trigger special effects when entering critical states
        if (CurrentState == ESanityState::Critical && PreviousState != ESanityState::Critical)
        {
            // Chance to trigger panic attack when first entering critical state
            if (FMath::RandRange(0.0f, 1.0f) < 0.4f)
            {
                TriggerPanicAttack();
            }
        }
    }
}

void USanityComponent::CheckRandomEvents()
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float DeltaTime = UpdateFrequency;

    // Check for panic attacks (only in critical state)
    if (CurrentState == ESanityState::Critical && CanTriggerPanicAttack())
    {
        float PanicRoll = FMath::RandRange(0.0f, 1.0f);
        float PanicThreshold = PanicAttackChance * DeltaTime;

        if (PanicRoll < PanicThreshold)
        {
            TriggerPanicAttack();
        }
    }

    // Check for hallucinations (in unstable or critical state)
    if ((CurrentState == ESanityState::Unstable || CurrentState == ESanityState::Critical)
        && CanTriggerHallucination())
    {
        float HallucinationRoll = FMath::RandRange(0.0f, 1.0f);
        float HallucinationThreshold = HallucinationChance * DeltaTime;

        if (HallucinationRoll < HallucinationThreshold)
        {
            TriggerHallucination();
        }
    }
}

void USanityComponent::NotifyHeadbobComponent()
{
    if (HeadbobComponent)
    {
        HeadbobComponent->ForceUpdateHeadbob();
    }
}

void USanityComponent::HandleEnvironmentalDrain(float Value)
{
    if (!bEnvironmentalDrainActive || bInSafeZone || bSystemPaused) return;

    float CurrentDrainRate = GetCurrentDrainRate();
    float DeltaSanity = -Value * CurrentDrainRate * GetWorld()->GetDeltaSeconds();

    if (FMath::Abs(DeltaSanity) > 0.001f)
    {
        DrainSanity(FMath::Abs(DeltaSanity), false);
    }
}

float USanityComponent::GetCurrentDrainRate() const
{
    float BaseRate = EnvironmentalDrainRate;

    // Apply sanity modifiers
    BaseRate *= GetTotalSanityModifier();

    // Check if character is in darkness (assuming we have access to character's state)
    if (OwnerCharacter)
    {
        // Cast to our horror game character to check darkness state
        if (AHorrorGameCharacter* HorrorChar = Cast<AHorrorGameCharacter>(OwnerCharacter))
        {
            if (HorrorChar->IsInDarkness())
            {
                BaseRate *= DarknessDrainMultiplier;
            }

            // Additional multipliers based on fear level
            float FearLevel = HorrorChar->GetFearLevel();
            if (FearLevel > 0.3f)
            {
                BaseRate *= (1.0f + FearLevel * 0.5f); // Up to 50% increase
            }
        }
    }

    return BaseRate;
}

float USanityComponent::GetPostProcessIntensity() const
{
    const float SanityPercent = GetSanityPercent();

    if (SanityPercent > LowSanityEffectThreshold)
    {
        return 0.0f;
    }

    // Calculate intensity based on how far below threshold we are
    float IntensityRange = LowSanityEffectThreshold;
    float BelowThreshold = LowSanityEffectThreshold - SanityPercent;
    float BaseIntensity = BelowThreshold / IntensityRange;

    // Add some randomness for instability effect
    if (CurrentState >= ESanityState::Unstable)
    {
        float RandomVariation = FMath::RandRange(-0.1f, 0.1f);
        BaseIntensity += RandomVariation;
    }

    // Extra intensity during panic
    if (bInPanicAttack)
    {
        BaseIntensity *= 1.5f;
    }

    return FMath::Clamp(BaseIntensity, 0.0f, 1.0f);
}

bool USanityComponent::ValidateComponents()
{
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("SanityComponent: No owner character"));
        return false;
    }

    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("SanityComponent: No player controller found"));
    }

    return true;
}

bool USanityComponent::CanTriggerPanicAttack() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return !bInPanicAttack && (CurrentTime - LastPanicTime) >= PanicAttackCooldown;
}

bool USanityComponent::CanTriggerHallucination() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastHallucinationTime) >= HallucinationCooldown;
}

void USanityComponent::TriggerLowSanityPostProcess()
{
    if (!PostProcessComponent) return;

    // Enable full blend immediately
    PostProcessComponent->BlendWeight = 1.0f;

    const float FadeDuration = 2.0f;
    const float StartTime = GetWorld()->GetTimeSeconds();

    // Tạo delegate từ lambda (an toàn hơn so với truyền lambda thẳng)
    FTimerDelegate FadeDelegate = FTimerDelegate::CreateLambda([this, StartTime, FadeDuration]()
        {
            if (!PostProcessComponent || !GetWorld()) return;

            float Elapsed = GetWorld()->GetTimeSeconds() - StartTime;
            float Alpha = FMath::Clamp(1.0f - (Elapsed / FadeDuration), 0.0f, 1.0f);
            PostProcessComponent->BlendWeight = Alpha;

            PostProcessComponent->Settings.bOverride_SceneFringeIntensity = true;
            PostProcessComponent->Settings.SceneFringeIntensity = FMath::Lerp(6.0f, 0.0f, 1.0f - Alpha);

            PostProcessComponent->Settings.bOverride_MotionBlurAmount = true;
            PostProcessComponent->Settings.MotionBlurAmount = FMath::Lerp(1.0f, 0.0f, 1.0f - Alpha);

            // Nếu đã fade xong, tự clear timer
            if (Alpha <= 0.0f)
            {
                if (GetWorld())
                {
                    GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
                }
            }
        });

    // Khởi chạy timer (dùng FadeTimerHandle là biến thành viên)
    GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, FadeDelegate, 0.05f, true);

    // Backup: timer để đảm bảo dừng sau FadeDuration (không bắt handle cục bộ)
    FTimerDelegate StopDelegate = FTimerDelegate::CreateLambda([this]()
        {
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
            }
        });

    FTimerHandle KillHandle;
    GetWorld()->GetTimerManager().SetTimer(KillHandle, StopDelegate, FadeDuration + 0.1f, false);
}

void USanityComponent::OnSafeZoneExit()
{
    bInSafeZone = false;

    // Resume environmental drain when leaving safe zone
    if (!bEnvironmentalDrainActive)
    {
        StartEnvironmentalDrain();
    }

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Log, TEXT("Exited safe zone - environmental drain resumed"));
    }
}

void USanityComponent::AddSanityModifier(const FString& ModifierName, float ModifierValue)
{
    // Store temporary modifiers that affect drain rate
    if (!SanityModifiers.Contains(ModifierName))
    {
        SanityModifiers.Add(ModifierName, ModifierValue);

        if (bDebugSanity)
        {
            UE_LOG(LogTemp, Log, TEXT("Added sanity modifier '%s': %.2f"), *ModifierName, ModifierValue);
        }
    }
}

void USanityComponent::RemoveSanityModifier(const FString& ModifierName)
{
    if (SanityModifiers.Contains(ModifierName))
    {
        SanityModifiers.Remove(ModifierName);

        if (bDebugSanity)
        {
            UE_LOG(LogTemp, Log, TEXT("Removed sanity modifier '%s'"), *ModifierName);
        }
    }
}

float USanityComponent::GetTotalSanityModifier() const
{
    float TotalModifier = 1.0f;

    for (const auto& Modifier : SanityModifiers)
    {
        TotalModifier += Modifier.Value;
    }

    return FMath::Max(0.1f, TotalModifier); // Minimum 10% of normal rate
}

void USanityComponent::ResetSanityToMax()
{
    SetSanity(MaxSanity);

    // Reset all thresholds
    for (int32 i = 0; i < ThresholdTriggered.Num(); i++)
    {
        ThresholdTriggered[i] = false;
    }

    // Clear panic state
    bInPanicAttack = false;

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Log, TEXT("Sanity reset to maximum: %.2f"), MaxSanity);
    }
}

void USanityComponent::SetMaxSanity(float NewMaxSanity)
{
    float OldMaxSanity = MaxSanity;
    MaxSanity = FMath::Max(10.0f, NewMaxSanity); // Minimum 10 sanity

    // Adjust current sanity proportionally
    if (OldMaxSanity > 0.0f)
    {
        float SanityRatio = CurrentSanity / OldMaxSanity;
        CurrentSanity = MaxSanity * SanityRatio;
    }
    else
    {
        CurrentSanity = MaxSanity;
    }

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Log, TEXT("Max sanity changed: %.2f -> %.2f, Current: %.2f"),
            OldMaxSanity, MaxSanity, CurrentSanity);
    }
}

FString USanityComponent::GetSanityStateString() const
{
    switch (GetSanityState())
    {
    case ESanityState::Stable:
        return TEXT("Stable");
    case ESanityState::Nervous:
        return TEXT("Nervous");
    case ESanityState::Anxious:
        return TEXT("Anxious");
    case ESanityState::Unstable:
        return TEXT("Unstable");
    case ESanityState::Critical:
        return TEXT("Critical");
    default:
        return TEXT("Unknown");
    }
}

void USanityComponent::DebugPrintSanityInfo() const
{
    if (!bDebugSanity) return;

    FString DebugString = FString::Printf(
        TEXT("=== SANITY DEBUG INFO ===\n")
        TEXT("Current Sanity: %.2f/%.2f (%.1f%%)\n")
        TEXT("State: %s\n")
        TEXT("Environmental Drain: %s\n")
        TEXT("In Safe Zone: %s\n")
        TEXT("In Panic Attack: %s\n")
        TEXT("Drain Rate: %.2f\n")
        TEXT("Post Process Intensity: %.2f\n")
        TEXT("Last Panic: %.2f seconds ago\n")
        TEXT("Last Hallucination: %.2f seconds ago\n"),
        CurrentSanity, MaxSanity, GetSanityPercent() * 100.0f,
        *GetSanityStateString(),
        bEnvironmentalDrainActive ? TEXT("Active") : TEXT("Inactive"),
        bInSafeZone ? TEXT("Yes") : TEXT("No"),
        bInPanicAttack ? TEXT("Yes") : TEXT("No"),
        GetCurrentDrainRate(),
        GetPostProcessIntensity(),
        GetWorld()->GetTimeSeconds() - LastPanicTime,
        GetWorld()->GetTimeSeconds() - LastHallucinationTime
    );

    UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugString);

    // Also display on screen if GEngine is available
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugString);
    }
}