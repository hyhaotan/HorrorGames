#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Interface/Interact.h"
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
class HORRORGAME_API AElectronicLockActor : public AActor,public IInteract
{
    GENERATED_BODY()

public:
    AElectronicLockActor();
    virtual void BeginPlay() override;

    /** Add a digit to the entered code */
    void AddDigit(int32 Digit);

    /** Clear the entered code */
    void ClearEnteredCode() { EnteredCode.Empty(); }

    /** Verify entered code against correct code */
    void VerifyCode();

    virtual void Interact(AHorrorGameCharacter* Player) override;
    void UnInteract(AHorrorGameCharacter* Player);

    /** Delegate broadcast when code updates */
    UPROPERTY(BlueprintAssignable)
    FOnCodeUpdated OnCodeUpdated;

    /** Delegate broadcast on error */
    UPROPERTY(BlueprintAssignable)
    FOnCodeError OnCodeError;

protected:
    UPROPERTY(VisibleAnywhere)
    USphereComponent* SphereComponent;

    UPROPERTY(EditAnywhere, Category = "LockMesh")
    UStaticMeshComponent* LockMesh;

    UPROPERTY(EditAnywhere, Category = "Door")
    UStaticMeshComponent* Door;

    UPROPERTY(EditAnywhere, Category = "DoorFrame")
    UStaticMeshComponent* DoorFrame;

    UPROPERTY(VisibleAnywhere)
    UWidgetComponent* ItemWidget;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* LockCamera;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UElectronicLockWidget> ElectronicLockWidgetClass;

    UPROPERTY()
    UElectronicLockWidget* ElectronicLockWidget;

    /** Door actor to open */
    UPROPERTY(EditAnywhere)
    AActor* DoorActor;

    /** The correct sequence */
    UPROPERTY(EditAnywhere)
    TArray<int32> CorrectCode;

    /** Current entered sequence */
    TArray<int32> EnteredCode;

    /** Has the lock been opened? */
    bool bIsOpen = false;

private:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void OpenDoor();
};