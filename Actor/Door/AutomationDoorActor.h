#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "AutomationDoorActor.generated.h"

UCLASS()
class HORRORGAME_API AAutomationDoorActor : public AActor
{
	GENERATED_BODY()

public:
	AAutomationDoorActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOpenDoor();
	void MulticastOpenDoor_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastCloseDoor();
	void MulticastCloseDoor_Implementation();

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* DoorFrameMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* LeftDoorMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* RightDoorMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UCurveFloat* DoorOpenCurve;

	UPROPERTY()
	class UTimelineComponent* DoorTimeline;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UBoxComponent* TriggerBox;

	UPROPERTY(EditAnywhere, Category = "Door Settings")
	float OpenDistance = 100.0f;

	UPROPERTY(EditInstanceOnly,Replicated, Category = "Door")
	bool bIsOpen;

	int32 OverlapCount;

	void HandleDoorTimeline(float Value);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};