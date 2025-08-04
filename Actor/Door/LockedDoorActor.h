#pragma once

#include "CoreMinimal.h"
#include "HorrorGame/Actor/Door/DoorRootActor.h"
#include "LockedDoorActor.generated.h"

class UStaticMeshComponent;
class AHorrorGameCharacter;
class AKeys;

UCLASS()
class HORRORGAME_API ALockedDoorActor : public ADoorRootActor
{
    GENERATED_BODY()

public:
    ALockedDoorActor();

protected:
    virtual void BeginPlay() override;

    /** Lock mesh component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Lock")
    UStaticMeshComponent* LockMesh;

    /** Required key ID to unlock this door */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Lock")
    FName RequiredKeyID;

    /** Is door currently locked */
    UPROPERTY(ReplicatedUsing = OnRep_IsLocked, BlueprintReadOnly, Category = "Door|Lock")
    bool bIsLocked = true;

    /** Success message when door is unlocked */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Lock")
    FString UnlockSuccessMessage = TEXT("Mở khóa thành công!");

    /** RepNotify for lock state */
    UFUNCTION()
    void OnRep_IsLocked();

    /** Unlock door logic */
    UFUNCTION(BlueprintCallable, Category = "Door|Lock")
    void UnlockDoor(AHorrorGameCharacter* Player);

    /** Show key notification to player */
    UFUNCTION(BlueprintCallable, Category = "Door|Lock")
    void ShowKeyNotification(AHorrorGameCharacter* Player, const FString& Message);

    /** Override door opening conditions for locked door */
    virtual bool CanOpenDoor_Implementation(AHorrorGameCharacter* Player) override;

    /** Override interaction behavior for locked door */
    virtual void OnDoorInteraction_Implementation(AHorrorGameCharacter* Player) override;

    /** Check if player has the required key */
    UFUNCTION(BlueprintCallable, Category = "Door|Lock")
    bool PlayerHasRequiredKey(AHorrorGameCharacter* Player, AKeys*& OutKey);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};