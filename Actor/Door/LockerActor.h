#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Components/AudioComponent.h"
#include "HorrorGame/Actor/Door/DoorRootActor.h"
#include "LockerActor.generated.h"

UCLASS()
class HORRORGAME_API ALockerActor : public ADoorRootActor
{
    GENERATED_BODY()

public:
    ALockerActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Door mesh attached to pivot
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locker")
    UStaticMeshComponent* LockerDoorMesh;

    // Points for teleporting player in/out
    UPROPERTY(EditAnywhere, Category = "Locker|Setup")
    USceneComponent* EntryPoint;
    UPROPERTY(EditAnywhere, Category = "Locker|Setup")
    USceneComponent* ExitPoint;

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

private:
    virtual bool CanOpenDoor_Implementation(AHorrorGameCharacter* Player) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnDoorTimelineFinished();

    void PlayDoorSound(USoundBase* Sound);

    UFUNCTION()
	void OnRep_PlayerHidden();

    UPROPERTY(ReplicatedUsing = OnRep_PlayerHidden)
    bool bPlayerHidden;

    AHorrorGameCharacter* HiddenPlayer;
};