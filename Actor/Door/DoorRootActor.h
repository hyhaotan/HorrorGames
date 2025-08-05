#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Interface/Interact.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "Components/TimelineComponent.h"
#include "Engine/Engine.h"
#include "DoorRootActor.generated.h"

class UStaticMeshComponent;
class UCurveFloat;
class AHorrorGameCharacter;

UCLASS(BlueprintType, Blueprintable)
class HORRORGAME_API ADoorRootActor : public AInteractableActor, public IInteract
{
    GENERATED_BODY()

public:
    ADoorRootActor();

    /** Server RPC to handle door interaction */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerDoorInteract(AHorrorGameCharacter* Player);
    bool ServerDoorInteract_Validate(AHorrorGameCharacter* Player);
    void ServerDoorInteract_Implementation(AHorrorGameCharacter* Player);

    /** RepNotify for door state changes */
    UFUNCTION()
    void OnRep_DoorStateChanged();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Door pivot (hinge) component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    USceneComponent* DoorPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    class USceneComponent* DoorMeshPivot;

    /** Door mesh component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    UStaticMeshComponent* DoorMesh;

    /** Door frame component (optional) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    UStaticMeshComponent* DoorFrame;

    /** Timeline for door animation */
    UPROPERTY()
    UTimelineComponent* DoorTimeline;

    /** Custom timeline for advanced door behavior */
    UPROPERTY()
    FTimeline CustomDoorTimeline;

    /** Curve driving the door animation (0->1) */
    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    UCurveFloat* DoorOpenCurve;

    /** Door rotation settings */
    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    float DoorRotateAngle = 90.0f;

    /** Door rotations for closed and open states */
    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    FRotator ClosedRotation;

    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    FRotator OpenRotation;

    /** Door states - replicated */
    UPROPERTY(ReplicatedUsing = OnRep_DoorStateChanged)
    bool bIsDoorClosed = true;

    UPROPERTY(Replicated)
    bool bIsAnimating = false;

    UPROPERTY(Replicated)
    bool bDoorOnSameSide = false;

    UPROPERTY(Replicated)
    bool bDoorDirectionSet = false;

    /** Current player interacting with door */
    UPROPERTY()
    AHorrorGameCharacter* CurrentPlayer = nullptr;

    /** Timeline progress handler */
    UFUNCTION()
    void HandleDoorProgress(float Value);

    /** Timeline finished handler */
    UFUNCTION()
    void OnDoorAnimationFinished();

    /** Virtual functions that can be overridden by child classes */

    /** Check if door can be opened/closed */
    UFUNCTION(BlueprintNativeEvent, Category = "Door")
    bool CanOpenDoor(AHorrorGameCharacter* Player);
    virtual bool CanOpenDoor_Implementation(AHorrorGameCharacter* Player);

    /** Called when door interaction occurs - override this for custom behavior */
    UFUNCTION(BlueprintNativeEvent, Category = "Door")
    void OnDoorInteraction(AHorrorGameCharacter* Player);
    virtual void OnDoorInteraction_Implementation(AHorrorGameCharacter* Player);

    /** Play door animation - can be overridden */
    UFUNCTION(BlueprintNativeEvent, Category = "Door")
    void PlayDoorAnimation();
    virtual void PlayDoorAnimation_Implementation();

    /** Set door side based on player position */
    virtual void SetDoorSameSide();

    /** Calculate door rotation based on side and animation value */
    virtual FRotator CalculateDoorRotation(float AnimationValue);

    /** Setup timeline bindings */
    virtual void SetupTimeline();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    /** Interact interface implementation */
    virtual void Interact(AHorrorGameCharacter* Player) override;

    UFUNCTION(BlueprintCallable, Category = "Door")
    void SetDoorPivotPosition(FVector PivotOffsets);

    UFUNCTION(BlueprintCallable, Category = "Door")
    void AutoSetHingePivot(bool bLeftHinge = true);
};