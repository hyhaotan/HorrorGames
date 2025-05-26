#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "HorrorGame/Interface/Interact.h"
#include "Door.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UCurveFloat;
class AHorrorGameCharacter;
class UItemWidget;
class UWidgetComponent;
class USphereComponent;

UCLASS()
class HORRORGAME_API ADoor : public AActor,public IInteract
{
    GENERATED_BODY()

public:
    ADoor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    virtual void Interact(AHorrorGameCharacter* Player) override;

    /** Cached player character pointer */
    AHorrorGameCharacter* Players;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Widget")
    UWidgetComponent* ItemWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box Item")
    USphereComponent* SphereComponent;

    UPROPERTY()
    UItemWidget* PickupWidget;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
    TSubclassOf<UItemWidget> PickupWidgetClass;
private:
    /** Door frame mesh */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* DoorFrame;

    /** Actual door mesh that will rotate */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* Door;

    /** Float curve asset for the door animation timeline */
    UPROPERTY(EditAnywhere, Category = "Door")
    UCurveFloat* CurveFloat;

    /** Timeline for smooth door animation */
    FTimeline DoorTimeline;

    /** Rotation angle when opening the door */
    UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float DoorRotateAngle;

    /** State flag tracking whether the door is closed */
    bool bIsDoorClosed;

    /** Determines rotation direction based on character side */
    bool bDoorOnSameSide;

    /** Callback for timeline update driving the rotation */
    UFUNCTION()
    void OpenDoor(float Value);

    /** Determine if the player is on which side of the door */
    void SetDoorSameSide();


    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);
};