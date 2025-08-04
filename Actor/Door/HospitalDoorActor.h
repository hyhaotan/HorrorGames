#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Actor/Door/DoorRootActor.h"
#include "HospitalDoorActor.generated.h"

class UStaticMeshComponent;
class UTimelineComponent;
class UCurveFloat;
class AHorrorGameCharacter;
class ULevelSequence;

UCLASS()
class HORRORGAME_API AHospitalDoorActor : public ADoorRootActor
{
    GENERATED_BODY()

public:
    AHospitalDoorActor();

    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

    /** Curve for open animation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
    UCurveFloat* OpenCloseCurve;

    /** Sequence to play when unlocking (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level", meta = (AllowPrivateAccess = "true"))
    ULevelSequence* UnlockDoorSequence;

private:

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

    bool bHasOpened;

    UPROPERTY(ReplicatedUsing = OnRep_IsLocked)
    bool bIsLocked;

    /** RepNotify for visual opening (optional) */
    UFUNCTION()
    void OnRep_IsOpen();

    virtual bool CanOpenDoor_Implementation(AHorrorGameCharacter* Player) override;
};