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
class UItemWidget;

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

    /** Restore player movement and close UI */
    void EnableMovementPlayer(AHorrorGameCharacter* Player, bool bIsCanceled);

    /** Add a digit to the entered code */
    void AddDigit(int32 Digit);
    /** Verify entered code against correct code */
    void VerifyCode();
    /** Clear current code input */
    void ClearEnteredCode();
    /** Remove last entered digit */
    void DecreaseCode();

    /** Delegate broadcast when code updates */
    UPROPERTY(BlueprintAssignable)
    FOnCodeUpdated OnCodeUpdated;
    /** Delegate broadcast on error */
    UPROPERTY(BlueprintAssignable)
    FOnCodeError OnCodeError;

    /** Currently entered code by player */
    UPROPERTY(VisibleAnywhere)
    TArray<int32> EnteredCode;

    /** Get the correct randomly generated code */
    UFUNCTION(BlueprintCallable, Category = "Lock")
    const TArray<int32>& GetCorrectCode() const { return CorrectCode; }

    /** Randomly generated correct code */
    UPROPERTY(VisibleAnywhere, Category = "Lock")
    TArray<int32> CorrectCode;

protected:
    /** Base frame of door */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* DoorFrame;

    /** Door mesh that will animate */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* Door;

    /** Camera used when entering code */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UCameraComponent* LockCamera;

    /** UI widget class for code entry */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UElectronicLockWidget> ElectronicLockWidgetClass;

private:
    /** Instance of code UI */
    UElectronicLockWidget* ElectronicLockWidget = nullptr;

    /** Door opening timeline */
    UPROPERTY(EditAnywhere, Category = "Lock|Animation")
    UCurveFloat* DoorOpenCurve;
    UPROPERTY()
    class UTimelineComponent* DoorTimeline;

    /** Handle to clear code after delay */
    FTimerHandle ClearCodeHandle;

    /** Flag if door already open */
    bool bIsOpen = false;

    /** Stored initial transform for returning actor */
    FTransform InitialTransform;

    /** Cached player for post-open restore */
    AHorrorGameCharacter* PlayerCharacter = nullptr;

    /** Timeline callbacks */
    UFUNCTION()
    void HandleDoorProgress(float Value);
    UFUNCTION()
    void OnDoorTimelineFinished();

    /** Delay helper */
    void DelayClearCodeInput();
};
