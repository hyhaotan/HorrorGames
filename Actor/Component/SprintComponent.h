// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SprintComponent.generated.h"

// Delegate for UI updates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API USprintComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USprintComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Sprint();
	void UnSprint();

	// Delegate for UI updates
	UPROPERTY(BlueprintAssignable)
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(EditDefaultsOnly, Category = "HeadBob")
	float SprintSpeedThreshold = 300.f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	class AHorrorGameCharacter* OwnerCharacter;

	// Sprint state
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_SprintChanged, BlueprintReadWrite, Category = "Sprint")
	bool bIsSprint;

	// Stamina properties
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_StaminaChanged, BlueprintReadWrite, Category = "Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaSpringUsageRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRechargeRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	bool CanStaminaRecharge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float DelayForStaminaRecharge;

	// Movement properties
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MaxSprintSpeed = 600.f;

	// Speed update threshold to prevent jitter
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SpeedUpdateThreshold = 10.f;

	// Timer handle
	FTimerHandle StaminaRechargeTimerHandle;

private:
	// Helper functions
	float CalculateSprintSpeed() const;
	float CalculateSprintSpeed(float StaminaValue) const; // Overload for prediction
	void UpdateSprintSpeed();
	void HandleStaminaSprint(float DeltaTime);
	void HandleStaminaSprintPrediction(float DeltaTime); // Client-side prediction
	void EnableStaminaGain();
	void DepletedAllStamina();

	// RepNotify functions
	UFUNCTION()
	void OnRep_SprintChanged();

	UFUNCTION()
	void OnRep_StaminaChanged();

	// Server RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartSprint();
	bool Server_StartSprint_Validate() { return true; }
	void Server_StartSprint_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopSprint();
	bool Server_StopSprint_Validate() { return true; }
	void Server_StopSprint_Implementation();
};