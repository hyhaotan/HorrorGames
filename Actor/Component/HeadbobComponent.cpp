#include "HeadbobComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HorrorGame/Actor/Component/SprintComponent.h"
#include "HorrorGame/Actor/Component/SanityComponent.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"

UHeadbobComponent::UHeadbobComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize state
    CurrentHeadbobState = EHeadbobState::None;
    PreviousHeadbobState = EHeadbobState::None;
    bIsCurrentlyHeadbobbing = false;
    bHeadbobEnabled = true;

    // NEW: initialize enable flag for visual offsets
    bEnableHeadbob = true;

    // Initialize tracking variables
    CurrentCameraShake = nullptr;
    CurrentShakeClass = nullptr;
    CurrentIntensity = 0.0f;
    LastUpdateTime = 0.0f;
    LastMovementSpeed = 0.0f;
    BaseHeadbobIntensity = 1.0f;

    // Initialize custom headbob
    CustomShakeClass = nullptr;
    CustomIntensity = 0.0f;

    // Debug default
    bDebugHeadbob = false;
}

void UHeadbobComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("HeadbobComponent: Owner is not a Character"));
        SetComponentTickEnabled(false);
        return;
    }

    PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
    SprintComponent = OwnerCharacter->FindComponentByClass<USprintComponent>();
    SanityComponent = OwnerCharacter->FindComponentByClass<USanityComponent>();

    if (!ValidateComponents())
    {
        SetComponentTickEnabled(false);
        return;
    }

    CurrentHeadbobState = EHeadbobState::None;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UHeadbobComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bHeadbobEnabled || !ValidateComponents())
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        LastUpdateTime = CurrentTime;
        UpdateHeadbobState();
        ProcessHeadbobTransition();
    }
}

void UHeadbobComponent::StartHeadbob()
{
    bHeadbobEnabled = true;
    ForceUpdateHeadbob();
}

void UHeadbobComponent::StopHeadbob()
{
    ClearCurrentShake();
    CurrentHeadbobState = EHeadbobState::None;
    bIsCurrentlyHeadbobbing = false;

    if (bDebugHeadbob)
    {
        UE_LOG(LogTemp, Log, TEXT("Headbob manually stopped"));
    }
}

void UHeadbobComponent::ForceUpdateHeadbob()
{
    if (!bHeadbobEnabled) return;

    UpdateHeadbobState();
    ProcessHeadbobTransition();
}

void UHeadbobComponent::SetHeadbobEnabled(bool bEnabled)
{
    bHeadbobEnabled = bEnabled;

    if (!bEnabled)
    {
        StopHeadbob();
    }
    else
    {
        ForceUpdateHeadbob();
    }
}

void UHeadbobComponent::SetCustomHeadbob(TSubclassOf<UCameraShakeBase> CustomShake, float Intensity)
{
    CustomShakeClass = CustomShake;
    CustomIntensity = FMath::Clamp(Intensity, 0.1f, 3.0f);

    if (bIsCurrentlyHeadbobbing)
    {
        ForceUpdateHeadbob();
    }
}

void UHeadbobComponent::UpdateHeadbobState()
{
    EHeadbobState NewState = DetermineHeadbobState();

    if (NewState != CurrentHeadbobState)
    {
        PreviousHeadbobState = CurrentHeadbobState;
        CurrentHeadbobState = NewState;

        if (OnHeadbobStateChanged.IsBound())
        {
            OnHeadbobStateChanged.Broadcast(CurrentHeadbobState, PreviousHeadbobState);
        }

        if (bDebugHeadbob)
        {
            float Speed = OwnerCharacter->GetVelocity().Size2D();
            LogHeadbobChange(CurrentHeadbobState, Speed, CalculateIntensity());
        }
    }
}

void UHeadbobComponent::ProcessHeadbobTransition()
{
    if (CurrentHeadbobState == EHeadbobState::None)
    {
        if (bIsCurrentlyHeadbobbing)
        {
            ClearCurrentShake();
            bIsCurrentlyHeadbobbing = false;
        }
        return;
    }

    if (!ShouldPlayHeadbob())
    {
        if (bIsCurrentlyHeadbobbing)
        {
            ClearCurrentShake();
            bIsCurrentlyHeadbobbing = false;
        }
        return;
    }

    ApplyHeadbob();
}

void UHeadbobComponent::ApplyHeadbob()
{
    if (!PlayerController) return;

    TSubclassOf<UCameraShakeBase> DesiredShake = GetShakeClassForState(CurrentHeadbobState);
    float DesiredIntensity = CalculateIntensity();

    if (!DesiredShake) return;

    bool bNeedsNewShake = !bIsCurrentlyHeadbobbing ||
        CurrentShakeClass != DesiredShake ||
        FMath::Abs(CurrentIntensity - DesiredIntensity) > 0.2f;

    if (bNeedsNewShake)
    {
        ClearCurrentShake();

        if (PlayerController->PlayerCameraManager)
        {
            CurrentCameraShake = PlayerController->PlayerCameraManager->StartCameraShake(DesiredShake, DesiredIntensity);
            CurrentShakeClass = DesiredShake;
            CurrentIntensity = DesiredIntensity;
            bIsCurrentlyHeadbobbing = true;

            if (bDebugHeadbob)
            {
                UE_LOG(LogTemp, Verbose, TEXT("Applied headbob: State=%d, Intensity=%.2f"),
                    (int32)CurrentHeadbobState, DesiredIntensity);
            }
        }
    }
}

void UHeadbobComponent::ClearCurrentShake()
{
    if (PlayerController && CurrentCameraShake && CurrentShakeClass)
    {
        PlayerController->ClientStopCameraShake(CurrentShakeClass, bSmoothTransitions);
    }

    CurrentCameraShake = nullptr;
    CurrentShakeClass = nullptr;
    CurrentIntensity = 0.0f;
}

EHeadbobState UHeadbobComponent::DetermineHeadbobState() const
{
    if (!ShouldPlayHeadbob()) return EHeadbobState::None;

    if (CustomShakeClass) return EHeadbobState::Custom;

    if (OwnerCharacter->bIsCrouched) return EHeadbobState::Crouching;

    if (SprintComponent && SprintComponent->IsSprinting()) return EHeadbobState::Sprinting;

    if (OwnerCharacter->GetVelocity().Size2D() <= 0.0f) return EHeadbobState::Idle;

    return EHeadbobState::Walking;
}

bool UHeadbobComponent::ShouldPlayHeadbob() const
{
    if (!OwnerCharacter) return false;

    const float Speed = OwnerCharacter->GetVelocity().Size2D();

    if (Speed < MinSpeedForHeadbob) return false;

    if (!OwnerCharacter->CanJump()) return false;

    if (AHorrorGameCharacter* HorrorChar = Cast<AHorrorGameCharacter>(OwnerCharacter))
    {
        if (HorrorChar->IsKnockedDown()) return false;
    }

    return true;
}

float UHeadbobComponent::CalculateIntensity() const
{
    if (CurrentHeadbobState == EHeadbobState::Custom)
    {
        return CustomIntensity;
    }

    float BaseIntensity = GetBaseIntensityForState(CurrentHeadbobState);
    const float Speed = OwnerCharacter->GetVelocity().Size2D();

    float SpeedMultiplier = 1.0f;

    if (CurrentHeadbobState == EHeadbobState::Sprinting && SprintComponent)
    {
        SpeedMultiplier = Speed / FMath::Max(1.0f, SprintComponent->SprintSpeed);
    }
    else if (CurrentHeadbobState == EHeadbobState::Walking && SprintComponent)
    {
        SpeedMultiplier = Speed / FMath::Max(1.0f, SprintComponent->WalkSpeed);
    }
    else if (CurrentHeadbobState == EHeadbobState::Crouching)
    {
        SpeedMultiplier = FMath::Min(Speed / 150.0f, 1.0f);
    }

    SpeedMultiplier = FMath::Clamp(SpeedMultiplier, MinIntensityMultiplier, MaxIntensityMultiplier);

    return BaseIntensity * SpeedMultiplier;
}

TSubclassOf<UCameraShakeBase> UHeadbobComponent::GetShakeClassForState(EHeadbobState State) const
{
    switch (State)
    {
    case EHeadbobState::Walking:
        return WalkCameraShakeClass;
    case EHeadbobState::Sprinting:
        return SprintCameraShakeClass;
    case EHeadbobState::Crouching:
        return CrouchCameraShakeClass;
    case EHeadbobState::Custom:
        return CustomShakeClass;
    case EHeadbobState::Idle:
        return IdleCameraShakeClass;
    default:
        return nullptr;
    }
}

float UHeadbobComponent::GetBaseIntensityForState(EHeadbobState State) const
{
    switch (State)
    {
    case EHeadbobState::Walking:
        return WalkIntensity;
    case EHeadbobState::Sprinting:
        return SprintIntensity;
    case EHeadbobState::Crouching:
        return CrouchIntensity;
    case EHeadbobState::Custom:
        return CustomIntensity;
    case EHeadbobState::Idle:
        return IdleIntensity;
    default:
        return 0.0f;
    }
}

bool UHeadbobComponent::ValidateComponents()
{
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("HeadbobComponent: No owner character"));
        return false;
    }

    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("HeadbobComponent: No player controller - headbob disabled"));
        return false;
    }

    return true;
}

void UHeadbobComponent::LogHeadbobChange(EHeadbobState NewState, float Speed, float Intensity) const
{
    FString StateName;
    switch (NewState)
    {
    case EHeadbobState::None: StateName = TEXT("None"); break;
    case EHeadbobState::Walking: StateName = TEXT("Walking"); break;
    case EHeadbobState::Sprinting: StateName = TEXT("Sprinting"); break;
    case EHeadbobState::Crouching: StateName = TEXT("Crouching"); break;
    case EHeadbobState::Idle: StateName = TEXT("Idle"); break;
    case EHeadbobState::Custom: StateName = TEXT("Custom"); break;
    default: StateName = TEXT("Unknown"); break;
    }

    UE_LOG(LogTemp, Log, TEXT("Headbob State: %s | Speed: %.1f | Intensity: %.2f"),
        *StateName, Speed, Intensity);
}

float UHeadbobComponent::GetSanityModifier() const
{
    if (!SanityComponent) return 1.0f;

    float BaseModifier = 1.0f;
    ESanityState SanityState = SanityComponent->GetSanityState();

    switch (SanityState)
    {
    case ESanityState::Stable:
        BaseModifier = 1.0f;
        break;
    case ESanityState::Nervous:
        BaseModifier = 1.0f + (FMath::Sin(GetWorld()->GetTimeSeconds() * 8.0f) * 0.1f);
        break;
    case ESanityState::Anxious:
        BaseModifier = 1.0f + (FMath::Sin(GetWorld()->GetTimeSeconds() * 12.0f) * 0.15f);
        BaseModifier += FMath::RandRange(-0.05f, 0.05f);
        break;
    case ESanityState::Unstable:
    {
        float TimeSeconds = GetWorld()->GetTimeSeconds();
        BaseModifier = 1.0f + (FMath::Sin(TimeSeconds * 15.0f) * 0.2f);
        BaseModifier += (FMath::Sin(TimeSeconds * 23.0f) * 0.1f);
        BaseModifier += FMath::RandRange(-0.1f, 0.1f);
    }
    break;
    case ESanityState::Critical:
    {
        float Time = GetWorld()->GetTimeSeconds();
        BaseModifier = 1.0f;
        BaseModifier += FMath::Sin(Time * 20.0f) * 0.3f;
        BaseModifier += FMath::Sin(Time * 37.0f) * 0.2f;
        BaseModifier += FMath::Sin(Time * 51.0f) * 0.15f;
        BaseModifier += FMath::RandRange(-0.2f, 0.2f);

        if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
        {
            BaseModifier += FMath::RandRange(-0.5f, 0.5f);
        }
    }
    break;
    }

    if (SanityComponent->bInPanicAttack)
    {
        float PanicShake = FMath::Sin(GetWorld()->GetTimeSeconds() * 30.0f) * 0.4f;
        BaseModifier += PanicShake;
        BaseModifier += FMath::RandRange(-0.3f, 0.3f);
    }

    if (AHorrorGameCharacter* HorrorChar = Cast<AHorrorGameCharacter>(GetOwner()))
    {
        float FearLevel = HorrorChar->GetFearLevel();
        if (FearLevel > 0.5f)
        {
            float FearShake = (FearLevel - 0.5f) * 0.4f;
            BaseModifier += FearShake * FMath::Sin(GetWorld()->GetTimeSeconds() * 25.0f);
        }
    }

    return FMath::Clamp(BaseModifier, 0.1f, 3.0f);
}

FVector UHeadbobComponent::GetHeadbobOffset() const
{
    if (!bEnableHeadbob || !OwnerCharacter) return FVector::ZeroVector;

    FVector BaseOffset = CalculateBaseHeadbob();

    float SanityMod = GetSanityModifier();
    BaseOffset *= SanityMod;

    if (SanityComponent)
    {
        ESanityState State = SanityComponent->GetSanityState();

        if (State >= ESanityState::Anxious)
        {
            float TremorIntensity = (int32)State * 0.1f;
            float VerticalTremor = FMath::Sin(GetWorld()->GetTimeSeconds() * 40.0f) * TremorIntensity;
            BaseOffset.Z += VerticalTremor;
        }

        if (State >= ESanityState::Unstable)
        {
            float HorizontalIntensity = ((int32)State - 2) * 0.15f;
            float HorizontalShake = FMath::Sin(GetWorld()->GetTimeSeconds() * 33.0f) * HorizontalIntensity;
            BaseOffset.Y += HorizontalShake;
        }

        if (State == ESanityState::Critical)
        {
            if (FMath::RandRange(0.0f, 1.0f) < 0.05f)
            {
                FVector RandomJerk = FVector(
                    FMath::RandRange(-0.3f, 0.3f),
                    FMath::RandRange(-0.3f, 0.3f),
                    FMath::RandRange(-0.2f, 0.2f)
                );
                BaseOffset += RandomJerk;
            }
        }
    }

    return BaseOffset;
}

void UHeadbobComponent::UpdateHeadbobIntensity()
{
    if (!SanityComponent) return;

    float BaseSanityIntensity = 1.0f - SanityComponent->GetSanityPercent();

    ESanityState State = SanityComponent->GetSanityState();
    float StateMultiplier = 1.0f;

    switch (State)
    {
    case ESanityState::Stable:
        StateMultiplier = 1.0f;
        break;
    case ESanityState::Nervous:
        StateMultiplier = 1.2f;
        break;
    case ESanityState::Anxious:
        StateMultiplier = 1.5f;
        break;
    case ESanityState::Unstable:
        StateMultiplier = 2.0f;
        break;
    case ESanityState::Critical:
        StateMultiplier = 3.0f;
        break;
    }

    CurrentIntensity = BaseHeadbobIntensity * StateMultiplier;
    CurrentIntensity = FMath::Clamp(CurrentIntensity, 0.1f, 5.0f);
}

FVector UHeadbobComponent::CalculateBaseHeadbob() const
{
    if (!OwnerCharacter) return FVector::ZeroVector;

    const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const float Speed = OwnerCharacter->GetVelocity().Size2D();

    float Frequency = 0.0f;
    float Amplitude = 0.0f;

    switch (CurrentHeadbobState)
    {
    case EHeadbobState::Sprinting:
        Frequency = 9.0f + (Speed / 300.0f);
        Amplitude = SprintIntensity * BaseHeadbobIntensity * 0.03f;
        break;

    case EHeadbobState::Walking:
    {
        float WalkBase = SprintComponent ? SprintComponent->WalkSpeed : 300.0f;
        float t = FMath::Clamp(Speed / FMath::Max(1.0f, WalkBase), 0.0f, 1.5f);
        Frequency = FMath::Lerp(5.5f, 8.0f, t);
        Amplitude = WalkIntensity * BaseHeadbobIntensity * FMath::Lerp(0.015f, 0.025f, t);
    }
    break;

    case EHeadbobState::Crouching:
        Frequency = 3.5f;
        Amplitude = CrouchIntensity * BaseHeadbobIntensity * 0.01f;
        break;

    case EHeadbobState::Idle:
        Frequency = 0.8f;
        Amplitude = IdleIntensity * BaseHeadbobIntensity * 0.004f;
        break;

    case EHeadbobState::Custom:
        Frequency = 6.0f;
        Amplitude = CustomIntensity * BaseHeadbobIntensity * 0.02f;
        break;

    default:
        return FVector::ZeroVector;
    }

    float ForwardOffset = FMath::Sin(TimeSeconds * Frequency) * Amplitude;
    float RightOffset = FMath::Sin(TimeSeconds * Frequency * 0.5f + PI * 0.5f) * (Amplitude * 0.5f);
    float UpOffset = FMath::Abs(FMath::Sin(TimeSeconds * Frequency)) * (Amplitude * 1.5f);

    if (CurrentHeadbobState == EHeadbobState::Walking || CurrentHeadbobState == EHeadbobState::Sprinting)
    {
        float SpeedFactor = FMath::Clamp(Speed / (SprintComponent ? SprintComponent->SprintSpeed : 600.0f), 0.0f, 1.0f);
        ForwardOffset *= SpeedFactor;
        RightOffset *= SpeedFactor;
        UpOffset *= SpeedFactor;
    }

    ForwardOffset = FMath::Clamp(ForwardOffset, -MaxHeadbobOffset, MaxHeadbobOffset);
    RightOffset = FMath::Clamp(RightOffset, -MaxHeadbobOffset, MaxHeadbobOffset);
    UpOffset = FMath::Clamp(UpOffset, -MaxHeadbobOffset, MaxHeadbobOffset);

    return FVector(ForwardOffset, RightOffset, UpOffset);
}
