#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SprintComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSprintStateChanged, bool, bIsSprinting);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API USprintComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USprintComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Public interface
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StopSprint();

	UFUNCTION(BlueprintCallable, Category = "Sprint")
	bool CanSprint() const;

	UFUNCTION(BlueprintPure, Category = "Sprint")
	bool IsSprinting() const { return GetOwner()->HasAuthority() ? bIsSprinting : bPredictedSprinting; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetCurrentStamina() const { return GetOwner()->HasAuthority() ? CurrentStamina : PredictedStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaRatio() const {
		float stamina = GetCurrentStamina();
		return MaxStamina > 0 ? stamina / MaxStamina : 0.0f;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0", ClampMax = "500.0"))
	float WalkSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "300.0", ClampMax = "1000.0"))
	float SprintSpeed = 600.0f;
	
	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable)
	FOnSprintStateChanged OnSprintStateChanged;

protected:
	virtual void BeginPlay() override;

	// Stamina properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "50.0", ClampMax = "200.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentStamina, Category = "Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "5.0", ClampMax = "30.0"))
	float StaminaDrainRate = 12.0f; // Stamina per second while sprinting

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "8.0", ClampMax = "40.0"))
	float StaminaRegenRate = 18.0f; // Stamina per second when not sprinting

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float StaminaRegenDelay = 2.5f; // Seconds after stopping sprint before regen starts

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "5.0", ClampMax = "30.0"))
	float MinStaminaToSprint = 15.0f; // Minimum stamina needed to start sprinting

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float StaminaExhaustionThreshold = 3.0f; // When stamina drops below this, forced stop

	// Movement properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.2", ClampMax = "0.8"))
	float MinStaminaRatioForFullSpeed = 0.4f; // At what stamina ratio we get full sprint speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.5", ClampMax = "0.9"))
	float MinSpeedMultiplier = 0.7f; // Minimum speed multiplier when stamina is very low

	// Network properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float NetworkUpdateFrequency = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float StaminaBroadcastFrequency = 0.15f;

private:
	// Core replicated state
	UPROPERTY(ReplicatedUsing = OnRep_SprintState)
	bool bIsSprinting;

	UPROPERTY(ReplicatedUsing = OnRep_CanRegenStamina)
	bool bCanRegenStamina;

	UPROPERTY(Replicated)
	bool bIsExhausted; // New: Track exhaustion state

	// Internal state
	UPROPERTY()
	class AHorrorGameCharacter* OwnerCharacter;

	// Timers
	FTimerHandle StaminaRegenDelayTimer;

	// Network optimization
	float LastNetworkUpdateTime;
	float LastStaminaBroadcast;

	// Client prediction state
	float PredictedStamina;
	bool bPredictedSprinting;
	bool bPredictedExhausted;
	bool bPredictedCanRegen;

	// State tracking
	float LastValidStamina;
	bool bWasSprintingLastFrame;

	// Helper functions
	void UpdateStamina(float DeltaTime);
	void UpdateMovementSpeed();
	float CalculateCurrentSprintSpeed() const;
	void BroadcastStaminaChange();
	void BroadcastSprintStateChange();
	void StartStaminaRegeneration();
	void CheckExhaustionState();

	// Client prediction
	void HandleClientPrediction(float DeltaTime);
	void ReconcileWithServer();

	// Server RPC functions
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartSprint();
	bool ServerStartSprint_Validate() { return true; }
	void ServerStartSprint_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopSprint();
	bool ServerStopSprint_Validate() { return true; }
	void ServerStopSprint_Implementation();

	// RepNotify functions
	UFUNCTION()
	void OnRep_CurrentStamina();

	UFUNCTION()
	void OnRep_SprintState();

	UFUNCTION()
	void OnRep_CanRegenStamina();

	// Validation helpers
	bool CanStartSprint() const;
	bool ShouldForceStopSprint() const;
	bool IsMovingEnoughToSprint() const;
};