// Fill out your copyright notice in the Description page of Project Settings.

#include "SprintComponent.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

USprintComponent::USprintComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true); // Important for component replication

	// Initialize stamina values
	CurrentStamina = MaxStamina = 1.f;
	StaminaSpringUsageRate = 0.1f;
	StaminaRechargeRate = 0.1f;
	CanStaminaRecharge = true;
	DelayForStaminaRecharge = 2.f;

	// Initialize movement speeds
	WalkSpeed = 200.f;
	MaxSprintSpeed = 600.f;

	// Initialize sprint state
	bIsSprint = false;
}

void USprintComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the owner character
	OwnerCharacter = Cast<AHorrorGameCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("SprintComponent: Owner is not a HorrorGameCharacter!"));
		SetComponentTickEnabled(false);
		return;
	}

	// Set the default walk speed
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Initialize UI if we're on the client
	if (OnStaminaChanged.IsBound())
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void USprintComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Handle stamina on both server and client for smooth prediction
	if (GetOwner()->HasAuthority())
	{
		HandleStaminaSprint(DeltaTime);
	}
	else
	{
		// Client-side prediction for smooth experience
		HandleStaminaSprintPrediction(DeltaTime);
	}
}

void USprintComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate stamina less frequently to reduce network traffic during speed transitions
	DOREPLIFETIME_CONDITION(USprintComponent, CurrentStamina, COND_SkipOwner);
	DOREPLIFETIME(USprintComponent, bIsSprint);

	// Add custom replication for smoother experience
	FDoRepLifetimeParams StaminaParams;
	StaminaParams.Condition = COND_SkipOwner;
	StaminaParams.RepNotifyCondition = REPNOTIFY_OnChanged;
	DOREPLIFETIME_WITH_PARAMS_FAST(USprintComponent, CurrentStamina, StaminaParams);
}

void USprintComponent::HandleStaminaSprint(float DeltaTime)
{
	if (!OwnerCharacter) return;

	// Don't handle stamina if crouched
	if (OwnerCharacter->bIsCrouched)
	{
		return;
	}

	float OldStamina = CurrentStamina;
	float OldSprintSpeed = bIsSprint ? CalculateSprintSpeed(CurrentStamina) : WalkSpeed;

	if (bIsSprint)
	{
		// Consume stamina while sprinting
		CurrentStamina = FMath::Clamp(CurrentStamina - (StaminaSpringUsageRate * DeltaTime), 0.f, MaxStamina);

		if (CurrentStamina <= 0.f)
		{
			DepletedAllStamina();
		}
		else
		{
			// Update speed only if there's a significant change to avoid micro-adjustments
			float NewSprintSpeed = CalculateSprintSpeed(CurrentStamina);
			if (FMath::Abs(OldSprintSpeed - NewSprintSpeed) > 10.f) // 10 unit threshold
			{
				UpdateSprintSpeed();
			}
		}
	}
	else
	{
		// Regenerate stamina when not sprinting
		if (CurrentStamina < MaxStamina && CanStaminaRecharge)
		{
			CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRechargeRate * DeltaTime), 0.f, MaxStamina);
		}
	}

	// Broadcast UI update if stamina changed significantly
	if (FMath::Abs(OldStamina - CurrentStamina) > 0.01f)
	{
		if (OnStaminaChanged.IsBound())
		{
			OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
		}
	}
}

void USprintComponent::HandleStaminaSprintPrediction(float DeltaTime)
{
	if (!OwnerCharacter) return;

	// Client-side prediction - same logic as server but only for local player
	if (!OwnerCharacter->IsLocallyControlled()) return;

	// Don't handle stamina if crouched
	if (OwnerCharacter->bIsCrouched)
	{
		return;
	}

	if (bIsSprint)
	{
		float OldStamina = CurrentStamina;
		float OldSpeed = CalculateSprintSpeed(CurrentStamina);

		// Predict stamina consumption
		float PredictedStamina = FMath::Clamp(CurrentStamina - (StaminaSpringUsageRate * DeltaTime), 0.f, MaxStamina);

		// Only update speed if there's a significant change
		float NewSpeed = CalculateSprintSpeed(PredictedStamina);
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	}

	// Update UI for local player - less frequent updates to reduce network traffic
	static float UIUpdateTimer = 0.f;
	UIUpdateTimer += DeltaTime;
	if (UIUpdateTimer >= 0.1f) // Update UI every 0.1 seconds
	{
		UIUpdateTimer = 0.f;
		if (OnStaminaChanged.IsBound())
		{
			OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
		}
	}
}

void USprintComponent::DepletedAllStamina()
{
	UnSprint();
}

void USprintComponent::OnRep_SprintChanged()
{
	UpdateSprintSpeed();
}

void USprintComponent::OnRep_StaminaChanged()
{
	// Update UI on clients when stamina changes
	if (OnStaminaChanged.IsBound())
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}

	// Update speed when stamina replicates
	if (bIsSprint)
	{
		UpdateSprintSpeed();
	}
}

void USprintComponent::UpdateSprintSpeed()
{
	if (!OwnerCharacter) return;

	float DesiredSpeed = bIsSprint ? CalculateSprintSpeed() : WalkSpeed;
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = DesiredSpeed;
}

float USprintComponent::CalculateSprintSpeed() const
{
	return CalculateSprintSpeed(CurrentStamina);
}

float USprintComponent::CalculateSprintSpeed(float StaminaValue) const
{
	if (StaminaValue >= 0.4f)
	{
		return MaxSprintSpeed;
	}

	// Use step-based reduction instead of linear interpolation to reduce jitter
	float StaminaRatio = StaminaValue / 0.4f;

	// Create speed tiers to reduce micro-adjustments
	if (StaminaRatio >= 0.75f) 
	{
		return FMath::Lerp(MaxSprintSpeed * 0.9f, MaxSprintSpeed, (StaminaRatio - 0.75f) / 0.25f);
	}
	else if (StaminaRatio >= 0.5f) 
	{
		return FMath::Lerp(MaxSprintSpeed * 0.75f, MaxSprintSpeed * 0.9f, (StaminaRatio - 0.5f) / 0.25f);
	}
	else if (StaminaRatio >= 0.25f)
	{
		return FMath::Lerp(MaxSprintSpeed * 0.6f, MaxSprintSpeed * 0.75f, (StaminaRatio - 0.25f) / 0.25f);
	}
	else
	{
		return FMath::Lerp(WalkSpeed, MaxSprintSpeed * 0.6f, StaminaRatio / 0.25f);
	}
}

void USprintComponent::EnableStaminaGain()
{
	CanStaminaRecharge = true;
}

void USprintComponent::Server_StartSprint_Implementation()
{
	if (!OwnerCharacter) return;

	// Check if we can sprint
	if (OwnerCharacter->bIsKnockedDown ||
		OwnerCharacter->bIsCrouched ||
		OwnerCharacter->GetVelocity().IsNearlyZero() ||
		CurrentStamina <= 0.f)
	{
		return;
	}

	bIsSprint = true;
	UpdateSprintSpeed();

	// Stop stamina recharge while sprinting
	CanStaminaRecharge = false;
	GetWorld()->GetTimerManager().ClearTimer(StaminaRechargeTimerHandle);
}

void USprintComponent::Server_StopSprint_Implementation()
{
	if (!bIsSprint) return;

	bIsSprint = false;
	UpdateSprintSpeed();

	// Start timer to enable stamina recharge after delay
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			StaminaRechargeTimerHandle,
			this,
			&USprintComponent::EnableStaminaGain,
			DelayForStaminaRecharge,
			false
		);
	}
}

void USprintComponent::Sprint()
{
	if (!OwnerCharacter) return;

	// Immediate local response for better feel
	if (OwnerCharacter->IsLocallyControlled())
	{
		// Check conditions locally first
		if (OwnerCharacter->bIsKnockedDown ||
			OwnerCharacter->bIsCrouched ||
			OwnerCharacter->GetVelocity().IsNearlyZero() ||
			CurrentStamina <= 0.f)
		{
			return;
		}

		// Predict sprint state locally for immediate response
		if (!bIsSprint)
		{
			bIsSprint = true;
			UpdateSprintSpeed();
			CanStaminaRecharge = false;
		}
	}

	if (OwnerCharacter->HasAuthority())
	{
		// Call implementation directly on server
		Server_StartSprint_Implementation();
	}
	else
	{
		// Call RPC on client
		Server_StartSprint();
	}
}

void USprintComponent::UnSprint()
{
	if (!OwnerCharacter) return;

	// Immediate local response
	if (OwnerCharacter->IsLocallyControlled() && bIsSprint)
	{
		bIsSprint = false;
		UpdateSprintSpeed();
	}

	if (OwnerCharacter->HasAuthority())
	{
		// Call implementation directly on server
		Server_StopSprint_Implementation();
	}
	else
	{
		// Call RPC on client
		Server_StopSprint();
	}
}