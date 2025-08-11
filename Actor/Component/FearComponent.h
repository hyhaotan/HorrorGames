#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FearComponent.generated.h"

class AHorrorGameCharacter;
class UCurveFloat;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API UFearComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFearComponent();

    UFUNCTION(BlueprintCallable, Category = "Fear")
    float GetFearLevel() const;

protected:
    virtual void BeginPlay() override;

    float BaseSanityFear() const;

    float ChaseFear() const;

    float DarknessFear() const;

    float ThreatProximityFear() const;

protected:
    UPROPERTY()
    AHorrorGameCharacter* OwnerChar;

    UPROPERTY()
    class USanityComponent* SanityComp;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear")
    UCurveFloat* FearCurve;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear")
    float ChaseFearAmount = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear")
    float DarknessFearAmount = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear")
    float ThreatRadius = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear")
    float ThreatProximityScale = 1.f;
};
