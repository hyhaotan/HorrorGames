#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EyeMonster.generated.h"

class USkeletalMeshComponent;
class UParticleSystemComponent;
class UBoxComponent;
class UCameraShakeBase;
class UUserWidget;
class USoundBase;

UCLASS()
class HORRORGAME_API AEyeMonster : public AActor
{
    GENERATED_BODY()

public:
    AEyeMonster();
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void DoBlink();

    void SetIlluminated(bool bOn);

    bool CanDamage() const { return !bIsCoveringEyes; }

protected:
    virtual void BeginPlay() override;

private:
    /** Root scene component */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* SceneRoot;

    /** Skeletal mesh for visual representation */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USkeletalMeshComponent* MeshComponent;

    /** Aura particle effect */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UParticleSystemComponent* AuraEffect;

    /** Defines the volume within which the monster can spawn */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* SpawnVolume;

    /** Sound played when inflicting damage */
    UPROPERTY(EditAnywhere, Category = "Effects|Sound")
    USoundBase* DamageScreamSound;

    /** Camera shake on damage */
    UPROPERTY(EditAnywhere, Category = "Effects|Camera")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    /** Blood overlay UI widget class */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> BloodOverlayClass;

    UPROPERTY(EditDefaultsOnly, Category = "Hallucination")
    TSubclassOf<AActor> HallucinationActorClass;

    UPROPERTY(EditAnywhere, Category = "Animation")
    UAnimMontage* CoverEyesMontage = nullptr;

    /** Duration the blood overlay remains on-screen */
    UPROPERTY(EditAnywhere, Category = "UI")
    float BloodOverlayDuration = 0.5f;

    /** Runtime instance of the blood overlay */
    UPROPERTY()
    UUserWidget* BloodOverlayWidget = nullptr;

    /** Damage dealt per interval when looked at */
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float DamageValue = 5.f;

    /** Minimum dot product threshold to consider player "looking" */
    UPROPERTY(EditAnywhere, Category = "EyeMonster", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LookThreshold = 0.8f;

    /** Height above which traces start */
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float TraceHeight = 1000.f;

    /** Depth below which traces end */
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float TraceDepth = 2000.f;

    /** Yaw offset when facing the player */
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float YawOffset = -90.f;

    float HallucinationAccumTime = 0.f;

    /** Smoothing speed for rotation (unused for instant facing) */
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float RotationSpeed = 2.5f;

    /** Time between each damage infliction when looking */
    UPROPERTY(EditAnywhere, Category = "EyeMonster")
    float LookDamageInterval = 1.0f;

    /** Accumulated time since last damage tick */
    float LookDamageTimer = 0.f;

    UPROPERTY(EditDefaultsOnly, Category = "Blink")
    float BlinkDuration = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Blink")
    float BlinkRadius = 500.f;

    UPROPERTY(EditAnywhere, Category = "Blink")
    float CoverEyesDuration = 2.0f;

    bool bIsBlinking = false;

    bool bIsCoveringEyes = false;

    /** Original spawn volume center in world space */
    FVector InitialSpawnCenter;

    /** Original spawn volume half-extents */
    FVector InitialSpawnExtent;

    /** Handle for self-destruction timer */
    FTimerHandle DestroyTimerHandle;

    /** Handle for respawn delegate timer */
    FTimerHandle RespawnTimerHandle;

    /** Handle for blood overlay removal timer */
    FTimerHandle BloodOverlayTimerHandle;

    FTimerHandle BlinkTimerHandle;

    /** Spawns the monster at a random ground location within the defined volume */
    void SpawnAtRandomLocation();

    /** Performs a vertical trace to find ground at XY and outputs the hit point */
    bool GetGroundSpawnLocation(const FVector2D& XY, FVector& OutLocation) const;

    /** Applies damage over time if the player is looking */
    void ApplyLookDamage(float DeltaTime);

    /** Checks if the player view is directed at this actor without obstruction */
    bool IsPlayerLookingAtMonster() const;

    /** Instantly faces the actor toward the player */
    void FacePlayer(float DeltaTime = 0.f);

    /** Inflicts damage, shows UI, camera shake, and sound */
    void InflictDamage();

    /** Safe helper to display and remove the blood overlay */
    void ShowBloodOverlay(APlayerController* PC);

    /** Safe helper to trigger camera shake */
    void ShakeCamera(APlayerController* PC) const;

    /** Safe helper to play damage sound */
    void PlayDamageSound() const;

    /** Removes the blood overlay widget */
    void RemoveBloodOverlay();

    /** Schedules self-destruction and respawn */
    void RespawnAndDestroy();

    /** Handles destruction, respawn delegate scheduling, and cleanup */
    void HandleSelfDestruct();

    void FinishBlink();

    void DrawSpawnDebug() const;
};
