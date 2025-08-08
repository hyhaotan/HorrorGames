#pragma once

#include "CoreMinimal.h"
#include "DoorRootActor.h"
#include "Door.generated.h"

class AHorrorGameCharacter;

UCLASS()
class HORRORGAME_API ADoor : public ADoorRootActor
{
    GENERATED_BODY()

public:
    ADoor();

protected:
    /** Override interaction behavior for flip-flop door */
    virtual void OnDoorInteraction_Implementation(AHorrorGameCharacter* Player) override;

    /** Override door opening conditions */
    virtual bool CanOpenDoor_Implementation(AHorrorGameCharacter* Player) override;

    /** Custom door animation for this door type */
    virtual void PlayDoorAnimation_Implementation() override;

    /** Custom rotation calculation if needed */
    virtual FRotator CalculateDoorRotation(float AnimationValue) override;

private:
};