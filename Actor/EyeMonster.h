#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EyeMonster.generated.h"

class USkeletalMeshComponent;
class UParticleSystemComponent;
class UBoxComponent;
class UCameraShakeBase;

UCLASS()
class HORRORGAME_API AEyeMonster : public AActor
{
    GENERATED_BODY()

public:
    AEyeMonster();

protected:
    virtual void BeginPlay() override;

    // Components
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USkeletalMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UParticleSystemComponent* AuraEffect;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* SpawnVolume;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects|Sound")
    USoundBase* DamageScreamSound;

    // Damage & Look
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float DamageValue;

    UPROPERTY(EditAnywhere, Category = "EyeMonster", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LookThreshold;

    // Raycast height/depth
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float TraceHeight;

    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float TraceDepth;

    UPROPERTY(EditAnywhere, Category = "Look")
    float YawOffset = -90.f;

    // Rotation
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float RotationSpeed;

    FVector InitialSpawnCenter;
    FVector InitialSpawnExtent;

    // Camera shake
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    // Timers
    FTimerHandle DamageTimerHandle;
    FTimerHandle DestroyTimerHandle;
    FTimerHandle RespawnTimerHandle;
	FTimerHandle LookDamageTimerHandle;

    // Helpers
    bool GetGroundSpawnLocation(const FVector2D& XY, FVector& OutLocation);
    void SetupSpawnLocation();

    void HandleLookDamage(float DeltaTime);
    void InflictDamage();
    void DestroySelf();
    void RotateToPlayer(float DeltaTime);
public:
    virtual void Tick(float DeltaTime) override;

    float LookDamageAccumulator = 0.f;
};
