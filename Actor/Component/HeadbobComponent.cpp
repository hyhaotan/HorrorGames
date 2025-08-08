#include "HorrorGame/Actor/Component/HeadbobComponent.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Component/SprintComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

UHeadbobComponent::UHeadbobComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize state
    CurrentHeadbobState = EHeadbobState::None;
    PreviousHeadbobState = EHeadbobState::None;
    bIsCurrentlyHeadbobbing = false;
    bHeadbobEnabled = true;

    // Initialize tracking variables
    CurrentCameraShake = nullptr;
    CurrentShakeClass = nullptr;
    CurrentIntensity = 0.0f;
    LastUpdateTime = 0.0f;
    LastSpeedCheck = 0.0f;
    LastMovementSpeed = 0.0f;

    // Initialize custom headbob
    CustomShakeClass = nullptr;
    CustomIntensity = 0.0f;
}

void UHeadbobComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("HeadbobComponent: Owner is not a Character"));
        SetComponentTickEnabled(false);
        return;
    }

    PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
    SprintComponent = OwnerCharacter->FindComponentByClass<USprintComponent>();

    if (!ValidateComponents())
    {
        SetComponentTickEnabled(false);
        return;
    }

    // Initialize state
    CurrentHeadbobState = EHeadbobState::None;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UHeadbobComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bHeadbobEnabled || !ValidateComponents())
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Throttle updates for performance
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

    // If we're currently in a headbob state, apply the custom one
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

        // Broadcast state change
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

    // Apply headbob for current state
    ApplyHeadbob();
}

void UHeadbobComponent::ApplyHeadbob()
{
    if (!PlayerController) return;

    TSubclassOf<UCameraShakeBase> DesiredShake = GetShakeClassForState(CurrentHeadbobState);
    float DesiredIntensity = CalculateIntensity();

    if (!DesiredShake) return;

    // Check if we need to change the current shake
    bool bNeedsNewShake = !bIsCurrentlyHeadbobbing ||
        CurrentShakeClass != DesiredShake ||
        FMath::Abs(CurrentIntensity - DesiredIntensity) > 0.2f;

    if (bNeedsNewShake)
    {
        // Clear existing shake
        ClearCurrentShake();

        // Start new shake
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

    // Custom headbob takes priority
    if (CustomShakeClass) return EHeadbobState::Custom;

    // Check character states
    if (OwnerCharacter->bIsCrouched) return EHeadbobState::Crouching;

    if (SprintComponent && SprintComponent->IsSprinting()) return EHeadbobState::Sprinting;

	if (OwnerCharacter->GetVelocity().Size2D() <= 0.0f) return EHeadbobState::Idle;

    return EHeadbobState::Walking;
}

bool UHeadbobComponent::ShouldPlayHeadbob() const
{
    if (!OwnerCharacter) return false;

    const float Speed = OwnerCharacter->GetVelocity().Size2D();

    // Must be moving fast enough
    if (Speed < MinSpeedForHeadbob) return false;

    // Must be on ground
    if (!OwnerCharacter->CanJump()) return false;

    // Don't headbob when knocked down (if this function exists)
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

    // Scale intensity based on speed
    float SpeedMultiplier = 1.0f;

    if (CurrentHeadbobState == EHeadbobState::Sprinting && SprintComponent)
    {
        SpeedMultiplier = Speed / SprintComponent->SprintSpeed;
    }
    else if (CurrentHeadbobState == EHeadbobState::Walking && SprintComponent)
    {
        SpeedMultiplier = Speed / SprintComponent->WalkSpeed;
    }
    else if (CurrentHeadbobState == EHeadbobState::Crouching)
    {
        // Crouch speed is typically slower
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
        // This might be normal for AI characters
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
    case EHeadbobState::Custom: StateName = TEXT("Custom"); break;
    default: StateName = TEXT("Unknown"); break;
    }

    UE_LOG(LogTemp, Log, TEXT("Headbob State: %s | Speed: %.1f | Intensity: %.2f"),
        *StateName, Speed, Intensity);
} 