// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Interface/Interact.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "Components/TimelineComponent.h"
#include "DoorRootActor.generated.h"

class UStaticMeshComponent;
class UCurveFloat;
class AHorrorGameCharacter;

UCLASS()
class HORRORGAME_API ADoorRootActor : public AInteractableActor, public IInteract
{
    GENERATED_BODY()
    
public:    
    // Sets default values for this actor's properties
    ADoorRootActor();

    /** Server RPC to handle Interact */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerInteract(AHorrorGameCharacter* Player);
    bool ServerInteract_Validate(AHorrorGameCharacter* Player);
    void ServerInteract_Implementation(AHorrorGameCharacter* Player);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    /** Door pivot (hinge) component */
    UPROPERTY(EditAnywhere, Category = "Door")
    USceneComponent* DoorPivot;

    /** Door mesh component */
    UPROPERTY(EditAnywhere, Category = "Door")
    UStaticMeshComponent* DoorMesh;

    /** Timeline for door animation */
    UPROPERTY()
    UTimelineComponent* DoorTimeline;

    /** Curve driving the open animation (0->1) */
    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    UCurveFloat* DoorOpenCurve;

    /** Door angles for closed and open states */
    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    FRotator ClosedRotation;

    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    FRotator OpenRotation;

    /** Handle timeline progress */
    UFUNCTION()
    void HandleDoorProgress(float Value);

    /** Play door open animation */
    virtual void PlayOpenDoorAnim();

    /** Virtual function to check if door can be opened */
    UFUNCTION(BlueprintNativeEvent, Category = "Door")
    bool CanOpenDoor(AHorrorGameCharacter* Player);
    virtual bool CanOpenDoor_Implementation(AHorrorGameCharacter* Player);

    /** Door opened state */
    UPROPERTY(Replicated)
    bool bHasOpened;  // true when door open animation has played

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

private:
    /** Interact interface implementation */
    virtual void Interact(AHorrorGameCharacter* Player) override;
};
