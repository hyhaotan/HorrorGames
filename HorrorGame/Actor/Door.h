#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Components/BoxComponent.h"
#include "HorrorGame/Interface/Interact.h"
#include "Door.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;

UCLASS()
class HORRORGAME_API ADoor : public AActor, public IInteract
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Implement the Interact function from the IInteract interface
	virtual void Interact() override;

	// Reference to the player
	class AHorrorGameCharacter* Player;
private:
	// Door frame mesh
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* DoorFrame;

	// Door mesh
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Door;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* REFInside;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* REFOutside;

	// Box collision for interaction
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* BoxCollision;

	// Timeline for door animation
	UPROPERTY(VisibleAnywhere, Category = "Timeline")
	FTimeline Timeline;

	// Curve for door rotation
	UPROPERTY(EditAnywhere, Category = "Timeline")
	class UCurveFloat* CurveFloat;

	// Is the door currently closed
	bool bIsDoorClose = true;

	// Door rotation angle
	UPROPERTY(EditAnywhere, Category = "Door")
	float DoorRotateAngle = 90.f;

	// Whether the player is on the same side of the door
	bool bDoorOnSameSide;

	bool bIsPlayerInside;

	// Function to animate door rotation
	UFUNCTION()
	void OpenDoor(float Value);

	// Determine if the door should rotate in the same direction
	void SetDoorSameSide();
};
