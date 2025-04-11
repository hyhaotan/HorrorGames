#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireZone.generated.h"

class UBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class AHorrorGameCharacter;

UCLASS()
class HORRORGAME_API AFireZone : public AActor
{
    GENERATED_BODY()

public:
    AFireZone();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* CollisionBox;

    UPROPERTY(EditDefaultsOnly, Category = "Fire")
    float BurnDamage;

    UPROPERTY(EditDefaultsOnly, Category = "Fire")
    float BurnDamageInterval;

    UPROPERTY(EditDefaultsOnly, Category = "Fire")
    UNiagaraSystem* BurningEffect;

    UPROPERTY()
    TMap<AActor*, UNiagaraComponent*> ActiveBurningEffects;

    UPROPERTY()
    TMap<AActor*, FTimerHandle> DamageTimerMap;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void StartBurnDamage(AHorrorGameCharacter* Character);
    void RemoveBurningEffect(AHorrorGameCharacter* Character, UNiagaraComponent* NiagaraComp);
    void ApplyBurnDamage(AActor* AffectedActor);
};
