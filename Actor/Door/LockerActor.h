#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Components/AudioComponent.h"
#include "HorrorGame/Interface/Interact.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "LockerActor.generated.h"

UCLASS()
class HORRORGAME_API ALockerActor : public AInteractableActor, public IInteract
{
    GENERATED_BODY()

public:
    ALockerActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Pivot for proper hinge behavior
    UPROPERTY(EditAnywhere, Category = "Locker|Setup")
    USceneComponent* DoorPivot;

    // Door mesh attached to pivot
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locker")
    UStaticMeshComponent* LockerDoorMesh;

    // Points for teleporting player in/out
    UPROPERTY(EditAnywhere, Category = "Locker|Setup")
    USceneComponent* EntryPoint;
    UPROPERTY(EditAnywhere, Category = "Locker|Setup")
    USceneComponent* ExitPoint;

    // Curve asset driving the door open/close
    UPROPERTY(EditAnywhere, Category = "Locker|Setup")
    UCurveFloat* DoorOpenCurve;

    // Timeline for smooth door animation
    UPROPERTY()
    UTimelineComponent* DoorTimeline;

    // Closed and open rotations
    FRotator DoorClosedRotation;
    FRotator DoorOpenRotation;

    // Audio component for door sounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* DoorAudioComp;

    // Sounds
    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* OpenDoorSound;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* CloseDoorSound;

    // Timeline tick callback
    UFUNCTION()
    void HandleDoorProgress(float Value);

    // Timeline finished callback
    UFUNCTION()
    void OnDoorTimelineFinished();

    // Helper to play sound
    void PlayDoorSound(USoundBase* Sound);

private:
    virtual void Interact(AHorrorGameCharacter* Player) override;

    bool bPlayerHidden;
    AHorrorGameCharacter* HiddenPlayer;
};