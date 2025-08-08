#include "HorrorGame/Actor/Component/SprintComponent.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

USprintComponent::USprintComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// Initialize stamina
	CurrentStamina = MaxStamina;
	bIsSprinting = false;
	bCanRegenStamina = true;
	bIsExhausted = false;

	// Client prediction
	PredictedStamina = MaxStamina;
	bPredictedSprinting = false;
	bPredictedExhausted = false;
	bPredictedCanRegen = true;

	// Network optimization
	LastNetworkUpdateTime = 0.0f;
	LastStaminaBroadcast = 0.0f;

	// State tracking
	LastValidStamina = MaxStamina;
	bWasSprintingLastFrame = false;
}

void USprintComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache owner character
	OwnerCharacter = Cast<AHorrorGameCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("SprintComponent: Owner is not a HorrorGameCharacter"));
		SetComponentTickEnabled(false);
		return;
	}

	// Set initial walk speed
	if (OwnerCharacter->GetCharacterMovement())
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	// Initialize stamina to full
	CurrentStamina = MaxStamina;
	PredictedStamina = MaxStamina;
	LastValidStamina = MaxStamina;

	// Broadcast initial state
	BroadcastStaminaChange();
	BroadcastSprintStateChange();
}

void USprintComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter)
		return;

	if (GetOwner()->HasAuthority())
	{
		// Server: Authoritative stamina and movement updates
		UpdateStamina(DeltaTime);
		CheckExhaustionState();
		UpdateMovementSpeed();

		// Force stop sprint if conditions are no longer met
		if (bIsSprinting && ShouldForceStopSprint())
		{
			ServerStopSprint();
		}

		// Network optimization: Only send updates at specified frequency
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastNetworkUpdateTime >= NetworkUpdateFrequency)
		{
			LastNetworkUpdateTime = CurrentTime;
		}
	}
	else if (OwnerCharacter->IsLocallyControlled())
	{
		// Client: Handle prediction for locally controlled character
		HandleClientPrediction(DeltaTime);
	}

	bWasSprintingLastFrame = bIsSprinting;
}

void USprintComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate core state
	DOREPLIFETIME_CONDITION(USprintComponent, CurrentStamina, COND_SkipOwner);
	DOREPLIFETIME(USprintComponent, bIsSprinting);
	DOREPLIFETIME(USprintComponent, bCanRegenStamina);
	DOREPLIFETIME(USprintComponent, bIsExhausted);
}

void USprintComponent::StartSprint()
{
	if (!OwnerCharacter)
		return;

	// Immediate local prediction for responsiveness
	if (OwnerCharacter->IsLocallyControlled() && CanStartSprint())
	{
		bPredictedSprinting = true;
		if (OwnerCharacter->GetCharacterMovement())
		{
			OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = CalculateCurrentSprintSpeed();
		}
		BroadcastSprintStateChange();
	}

	// Send to server regardless of authority (server will validate)
	if (!GetOwner()->HasAuthority())
	{
		ServerStartSprint();
	}
	else
	{
		ServerStartSprint();
	}
}

void USprintComponent::StopSprint()
{
	if (!OwnerCharacter)
		return;

	// Immediate local prediction
	if (OwnerCharacter->IsLocallyControlled())
	{
		bPredictedSprinting = false;
		if (OwnerCharacter->GetCharacterMovement())
		{
			OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}
		BroadcastSprintStateChange();
	}

	// Send to server
	if (!GetOwner()->HasAuthority())
	{
		ServerStopSprint();
	}
	else
	{
		ServerStopSprint();
	}
}

void USprintComponent::ServerStartSprint_Implementation()
{
	if (!CanStartSprint())
		return;

	bIsSprinting = true;
	bCanRegenStamina = false;

	// Clear any existing regeneration timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenDelayTimer);
	}

	UpdateMovementSpeed();
	BroadcastSprintStateChange();

	UE_LOG(LogTemp, Log, TEXT("Server: Started sprinting. Stamina: %.1f"), CurrentStamina);
}

void USprintComponent::ServerStopSprint_Implementation()
{
	if (!bIsSprinting)
		return;

	bIsSprinting = false;
	UpdateMovementSpeed();

	// Start regeneration delay timer only if we have some stamina left
	if (CurrentStamina > 0.0f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			StaminaRegenDelayTimer,
			this,
			&USprintComponent::StartStaminaRegeneration,
			StaminaRegenDelay,
			false
		);
	}

	BroadcastSprintStateChange();

	UE_LOG(LogTemp, Log, TEXT("Server: Stopped sprinting. Stamina: %.1f"), CurrentStamina);
}

bool USprintComponent::CanSprint() const
{
	return CanStartSprint();
}

bool USprintComponent::CanStartSprint() const
{
	if (!OwnerCharacter)
		return false;

	// Check if exhausted (need to recover more before sprinting again)
	if (bIsExhausted && CurrentStamina < MinStaminaToSprint * 1.5f)
		return false;

	// Check minimum stamina requirement
	if (CurrentStamina < MinStaminaToSprint)
		return false;

	// Check if character is crouched
	if (OwnerCharacter->bIsCrouched)
		return false;

	// Check if character is moving enough
	if (!IsMovingEnoughToSprint())
		return false;

	// Check if character is knocked down
	if (OwnerCharacter->IsKnockedDown())
		return false;

	return true;
}

bool USprintComponent::ShouldForceStopSprint() const
{
	// Force stop if stamina is completely depleted
	if (CurrentStamina <= StaminaExhaustionThreshold)
		return true;

	// Force stop if not moving enough
	if (!IsMovingEnoughToSprint())
		return true;

	// Force stop if crouched or knocked down
	if (OwnerCharacter && (OwnerCharacter->bIsCrouched || OwnerCharacter->IsKnockedDown()))
		return true;

	return false;
}

bool USprintComponent::IsMovingEnoughToSprint() const
{
	if (!OwnerCharacter)
		return false;

	return OwnerCharacter->GetVelocity().Size2D() > 50.0f; // Increased threshold for more realistic feel
}

void USprintComponent::UpdateStamina(float DeltaTime)
{
	float OldStamina = CurrentStamina;

	if (bIsSprinting)
	{
		// Only drain stamina if actually moving
		if (IsMovingEnoughToSprint())
		{
			CurrentStamina -= StaminaDrainRate * DeltaTime;
			CurrentStamina = FMath::Max(CurrentStamina, 0.0f);
		}
	}
	else if (bCanRegenStamina && !bIsExhausted)
	{
		// Regenerate stamina when not sprinting and not exhausted
		CurrentStamina += StaminaRegenRate * DeltaTime;
		CurrentStamina = FMath::Min(CurrentStamina, MaxStamina);
	}

	// Broadcast significant changes
	if (FMath::Abs(OldStamina - CurrentStamina) > 1.0f)
	{
		BroadcastStaminaChange();
	}
}

void USprintComponent::CheckExhaustionState()
{
	bool bWasExhausted = bIsExhausted;

	// Become exhausted when stamina hits the threshold
	if (CurrentStamina <= StaminaExhaustionThreshold && bIsSprinting)
	{
		bIsExhausted = true;
		bCanRegenStamina = false;

		// Force stop sprinting
		if (bIsSprinting)
		{
			bIsSprinting = false;
			UpdateMovementSpeed();
			BroadcastSprintStateChange();
		}

		// Longer delay when exhausted
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				StaminaRegenDelayTimer,
				this,
				&USprintComponent::StartStaminaRegeneration,
				StaminaRegenDelay * 1.5f, // 50% longer delay when exhausted
				false
			);
		}
	}
	// Recover from exhaustion when stamina is restored enough
	else if (bIsExhausted && CurrentStamina >= MinStaminaToSprint)
	{
		bIsExhausted = false;
	}

	// Log exhaustion state changes
	if (bWasExhausted != bIsExhausted)
	{
		UE_LOG(LogTemp, Log, TEXT("Exhaustion state changed: %s"), bIsExhausted ? TEXT("Exhausted") : TEXT("Recovered"));
	}
}

void USprintComponent::UpdateMovementSpeed()
{
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
		return;

	float TargetSpeed = bIsSprinting ? CalculateCurrentSprintSpeed() : WalkSpeed;
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

float USprintComponent::CalculateCurrentSprintSpeed() const
{
	if (!bIsSprinting)
		return WalkSpeed;

	// Calculate speed based on current stamina
	float StaminaRatio = GetStaminaRatio();

	if (StaminaRatio >= MinStaminaRatioForFullSpeed)
	{
		return SprintSpeed;
	}

	// Gradually reduce speed as stamina depletes
	float SpeedMultiplier = FMath::Max(MinSpeedMultiplier, StaminaRatio / MinStaminaRatioForFullSpeed);
	return FMath::Lerp(WalkSpeed, SprintSpeed, SpeedMultiplier);
}

void USprintComponent::HandleClientPrediction(float DeltaTime)
{
	if (!OwnerCharacter)
		return;

	// Predict stamina changes locally for smooth UI
	float OldPredictedStamina = PredictedStamina;

	if (bPredictedSprinting && CanStartSprint())
	{
		if (IsMovingEnoughToSprint())
		{
			PredictedStamina -= StaminaDrainRate * DeltaTime;
			PredictedStamina = FMath::Max(PredictedStamina, 0.0f);
		}

		// Check for predicted exhaustion
		if (PredictedStamina <= StaminaExhaustionThreshold)
		{
			bPredictedSprinting = false;
			bPredictedExhausted = true;
			bPredictedCanRegen = false;

			if (OwnerCharacter->GetCharacterMovement())
			{
				OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			}
		}
		else
		{
			// Update predicted speed
			float StaminaRatio = PredictedStamina / MaxStamina;
			float SpeedMultiplier = FMath::Max(MinSpeedMultiplier, StaminaRatio / MinStaminaRatioForFullSpeed);
			float PredictedSpeed = FMath::Lerp(WalkSpeed, SprintSpeed, SpeedMultiplier);

			if (OwnerCharacter->GetCharacterMovement())
			{
				OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = PredictedSpeed;
			}
		}
	}
	else if (!bPredictedSprinting && bPredictedCanRegen && !bPredictedExhausted)
	{
		PredictedStamina += StaminaRegenRate * DeltaTime;
		PredictedStamina = FMath::Min(PredictedStamina, MaxStamina);

		// Recover from predicted exhaustion
		if (bPredictedExhausted && PredictedStamina >= MinStaminaToSprint)
		{
			bPredictedExhausted = false;
		}
	}

	// Update UI with predicted values
	if (FMath::Abs(OldPredictedStamina - PredictedStamina) > 1.0f)
	{
		if (OnStaminaChanged.IsBound())
		{
			OnStaminaChanged.Broadcast(PredictedStamina, MaxStamina);
		}
	}

	// Reconcile with server occasionally
	static float ReconcileTimer = 0.0f;
	ReconcileTimer += DeltaTime;
	if (ReconcileTimer >= 0.5f)
	{
		ReconcileTimer = 0.0f;
		ReconcileWithServer();
	}
}

void USprintComponent::ReconcileWithServer()
{
	// Gradually adjust predicted values towards server values
	float StaminaDifference = CurrentStamina - PredictedStamina;
	if (FMath::Abs(StaminaDifference) > 5.0f)
	{
		PredictedStamina = FMath::FInterpTo(PredictedStamina, CurrentStamina, GetWorld()->GetDeltaSeconds(), 3.0f);
	}

	// Sync sprint state if different
	if (bPredictedSprinting != bIsSprinting)
	{
		bPredictedSprinting = bIsSprinting;
		UpdateMovementSpeed();
	}

	// Sync exhaustion state
	if (bPredictedExhausted != bIsExhausted)
	{
		bPredictedExhausted = bIsExhausted;
	}

	// Sync regeneration state
	if (bPredictedCanRegen != bCanRegenStamina)
	{
		bPredictedCanRegen = bCanRegenStamina;
	}
}

void USprintComponent::StartStaminaRegeneration()
{
	bCanRegenStamina = true;

	UE_LOG(LogTemp, Log, TEXT("Stamina regeneration started. Current: %.1f"), CurrentStamina);
}

void USprintComponent::BroadcastStaminaChange()
{
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Throttle broadcasts to avoid spam
	if (CurrentTime - LastStaminaBroadcast >= StaminaBroadcastFrequency)
	{
		LastStaminaBroadcast = CurrentTime;

		if (OnStaminaChanged.IsBound())
		{
			float BroadcastStamina = GetOwner()->HasAuthority() ? CurrentStamina : PredictedStamina;
			OnStaminaChanged.Broadcast(BroadcastStamina, MaxStamina);
		}
	}
}

void USprintComponent::BroadcastSprintStateChange()
{
	if (OnSprintStateChanged.IsBound())
	{
		bool CurrentSprintState = GetOwner()->HasAuthority() ? bIsSprinting : bPredictedSprinting;
		OnSprintStateChanged.Broadcast(CurrentSprintState);
	}
}

void USprintComponent::OnRep_CurrentStamina()
{
	// Validate replicated stamina
	LastValidStamina = CurrentStamina;
	BroadcastStaminaChange();

	if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled())
	{
		UpdateMovementSpeed();
	}
}

void USprintComponent::OnRep_SprintState()
{
	UpdateMovementSpeed();
	BroadcastSprintStateChange();
}

void USprintComponent::OnRep_CanRegenStamina()
{
	// Update prediction state for clients
	if (!GetOwner()->HasAuthority())
	{
		bPredictedCanRegen = bCanRegenStamina;
	}
}