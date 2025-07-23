﻿#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "HorrorGame/Interface/Interact.h"
#include "HospitalDoorActor.generated.h"

class UStaticMeshComponent;
class UTimelineComponent;
class UCurveFloat;
class AHorrorGameCharacter;
class ULevelSequence;

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
    bool ServerInteract_Validate(AHorrorGameCharacter* Player) { return true; }
    void ServerInteract_Implementation(AHorrorGameCharacter* Player);

protected:
    virtual void BeginPlay() override;

    /** Play the door open animation (Timeline) */
    void PlayOpenDoorAnim();

    /** Timeline float handler */
    void HandleOpenCloseDoor(float Value);

    /** Unlock with bolt cutter */
    void UnlockDoor(AHorrorGameCharacter* Player);

    /** RepNotify for locked */
    UFUNCTION()
    void OnRep_IsLocked();

    /** RepNotify for open sequence trigger */
    UFUNCTION()
    void OnRep_HasOpened();

    /** Curve for open animation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
    UCurveFloat* OpenCloseCurve;

    /** Sequence to play when unlocking (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level", meta = (AllowPrivateAccess = "true"))
    ULevelSequence* UnlockDoorSequence;

private:
    virtual void Interact(AHorrorGameCharacter* Player) override;

    /** Left and right door meshes */
    UPROPERTY(EditAnywhere, Category = "Door", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* LeftDoorMesh;

    UPROPERTY(EditAnywhere, Category = "Door", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* RightDoorMesh;

    /** Chain mesh that blocks the door */
    UPROPERTY(EditAnywhere, Category = "Door", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* ChainDoorMesh;

    /** Timeline component for opening */
    UPROPERTY(EditAnywhere, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
    UTimelineComponent* OpenCloseTimeline;

    /** Initial rotations */
    FRotator InitialLeftDoorRotation;
    FRotator InitialRightDoorRotation;

    /** Door state flags */
    UPROPERTY(ReplicatedUsing = OnRep_IsOpen)
    bool bIsOpen;

    UPROPERTY(ReplicatedUsing = OnRep_IsLocked)
    bool bIsLocked;

    UPROPERTY(ReplicatedUsing = OnRep_HasOpened)
    bool bHasOpened;

    /** RepNotify for visual opening (optional) */
    UFUNCTION()
    void OnRep_IsOpen();
};