#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Interface/Interact.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "ElectronicLockActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCodeUpdated, const TArray<int32>&, Entered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCodeError);

class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class UCameraComponent;
class AHorrorGameCharacter;
class UElectronicLockWidget;

UCLASS()
class HORRORGAME_API AElectronicLockActor : public AInteractableActor, public IInteract
{
    GENERATED_BODY()

public:
    AElectronicLockActor();
    virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

    /** Interact interface */
    virtual void Interact(AHorrorGameCharacter* Player) override;

	void EnableMovementPlayer(AHorrorGameCharacter* Player, bool bIsCanceled);

    /** Add a digit to the entered code */
    void AddDigit(int32 Digit);

    /** Verify entered code against correct code */
    void VerifyCode();

    /** Clear current code input */
    void ClearEnteredCode();

    /**Decrese code input**/
    void DecreseCode();

    /** Delegate broadcast when code updates */
    UPROPERTY(BlueprintAssignable)
    FOnCodeUpdated OnCodeUpdated;

    /** Delegate broadcast on error */
    UPROPERTY(BlueprintAssignable)
    FOnCodeError OnCodeError;

    TArray<int32> EnteredCode;

protected:

    UPROPERTY(EditAnywhere, Category = "Door")
    UStaticMeshComponent* Door;

    UPROPERTY(EditAnywhere, Category = "DoorFrame")
    UStaticMeshComponent* DoorFrame;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* LockCamera;

    /** UI class */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UElectronicLockWidget> ElectronicLockWidgetClass;

private:
    /** Widget instance */
    UElectronicLockWidget* ElectronicLockWidget = nullptr;

    /** Target door actor (optional) */
    UPROPERTY(EditAnywhere)
    AActor* DoorActor = nullptr;

    /** Correct code sequence */
    UPROPERTY(EditAnywhere)
    TArray<int32> CorrectCode;

    FTimerHandle ClearCodeHandle;

    bool bIsOpen = false;

    /** Curve asset để điều khiển góc quay của cửa */
    UPROPERTY(EditAnywhere, Category = "Lock|Animation")
    UCurveFloat* DoorOpenCurve;

    /** Timeline để tween góc quay */
    UPROPERTY()
    class UTimelineComponent* DoorTimeline;

    /** Hàm callback của timeline */
    UFUNCTION()
    void HandleDoorProgress(float Value);

    /** Đã bind xong timeline chưa */
    bool bTimelineInitialized;

    AHorrorGameCharacter* PlayerCharacter = nullptr;

    void OpenDoor(float InTargetYaw);

	void DelayClearCodeInput();

    UFUNCTION()
    void OnDoorTimelineFinished();
};