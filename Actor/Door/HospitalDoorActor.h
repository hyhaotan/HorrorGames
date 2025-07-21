#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "HorrorGame/Interface/Interact.h"
#include "HospitalDoorActor.generated.h"

class UStaticMeshComponent;
class UTimelineComponent;
class UCurveFloat;
class AHorrorGameCharacter;

UCLASS()
class HORRORGAME_API AHospitalDoorActor : public AInteractableActor, public IInteract
{
	GENERATED_BODY()

public:
	AHospitalDoorActor();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract(AHorrorGameCharacter* Player);
	bool ServerInteract_Validate(AHorrorGameCharacter* Player);
	void ServerInteract_Implementation(AHorrorGameCharacter* Player);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayOpenDoor();

	void UnlockDoor(AHorrorGameCharacter* Player);
	void HandleOpenCloseDoor(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	class ULevelSequence* UnlockDoorSequence;

private:
	virtual void Interact(AHorrorGameCharacter* Player) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LeftDoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* RightDoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ChainDoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* OpenCloseTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* OpenCloseCurve;

	FRotator InitialLeftDoorRotation;
	FRotator InitialRightDoorRotation;

	UPROPERTY(ReplicatedUsing = OnRep_IsOpen)
	bool bIsOpen;

	UFUNCTION()
	void OnRep_IsOpen();

	UPROPERTY(ReplicatedUsing = OnRep_IsLocked)
	bool bIsLocked;

	UFUNCTION()
	void OnRep_IsLocked();
};