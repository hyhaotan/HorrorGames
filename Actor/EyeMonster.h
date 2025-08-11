// EyeMonster.h - Enhanced Horror Version
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "EyeMonster.generated.h"

class USkeletalMeshComponent;
class UParticleSystemComponent;
class UBoxComponent;
class USceneComponent;
class USanityComponent;
class UUserWidget;
class USoundBase;
class UParticleSystem;
class UAnimMontage;
class UCameraShakeBase;

// Enum for monster behavior states
UENUM(BlueprintType)
enum class EEyeMonsterState : uint8
{
    Lurking     UMETA(DisplayName = "Lurking"),      // Hidden, waiting
    Staring     UMETA(DisplayName = "Staring"),      // Player is looking, building intensity
    Aggressive  UMETA(DisplayName = "Aggressive"),   // High damage mode
    Retreating  UMETA(DisplayName = "Retreating")    // Cooling down, preparing to hide
};

UCLASS()
class HORRORGAME_API AEyeMonster : public AActor
{
    GENERATED_BODY()

public:
    AEyeMonster();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* AuraEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* SpawnVolume;

    // === ENHANCED HORROR CONFIGURATION ===

    // Look Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Detection")
    float LookThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Spawning")
    float TraceHeight = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Spawning")
    float TraceDepth = 2000.f;

    // Sanity Damage System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Sanity Damage")
    float BaseSanityDamage = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Sanity Damage")
    float MaxSanityDamage = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Sanity Damage")
    float SanityDamageInterval = 0.8f;

    // Staring Mechanics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Staring")
    float StareIntensifyRate = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Staring")
    float MaxStareTime = 5.0f;

    // Proximity Fear
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Proximity")
    float ProximityFearRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Proximity")
    float ProximityFearDamage = 5.0f;

    // Blink/Teleport Mechanics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Teleportation")
    float BlinkCooldown = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Teleportation")
    float BlinkTeleportRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Settings|Teleportation")
    float BlinkRadius = 200.0f; // Legacy support

    // === VISUAL EFFECTS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects|UI")
    TSubclassOf<UUserWidget> BloodOverlayClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects|UI")
    float BloodOverlayDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects|Camera")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects|Particles")
    UParticleSystem* AggressiveAuraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects|Particles")
    UParticleSystem* BlinkParticleEffect;

    // === AUDIO SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Damage")
    USoundBase* DamageScreamSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Staring")
    USoundBase* StareStartSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Aggressive")
    USoundBase* AggressiveSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Movement")
    USoundBase* StealthBlinkSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Movement")
    USoundBase* EscapeBlinkSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Movement")
    USoundBase* RetreatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Movement")
    USoundBase* BlinkEffectSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    USoundBase* ProximityFearSound;

    // === ANIMATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* CoverEyesMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float CoverEyesDuration = 1.0f;

    // === ROTATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float YawOffset = 0.0f;

    // === PUBLIC INTERFACE ===

    UFUNCTION(BlueprintCallable, Category = "Monster Behavior")
    void DoBlink();

    UFUNCTION(BlueprintCallable, Category = "Monster Behavior")
    void SetIlluminated(bool bOn);

    UFUNCTION(BlueprintPure, Category = "Monster State")
    EEyeMonsterState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Monster State")
    bool IsPlayerStaring() const { return bIsStaring; }

    UFUNCTION(BlueprintPure, Category = "Monster State")
    float GetStareIntensity() const { return CurrentStareIntensity; }

    UFUNCTION(BlueprintPure, Category = "Monster State")
    float GetCurrentStareTime() const { return CurrentStareTime; }

protected:
    // === STATE MANAGEMENT ===

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EEyeMonsterState CurrentState = EEyeMonsterState::Lurking;

    // State Update Methods
    void UpdateBehaviorState(float DeltaTime);
    void HandleLurkingState(float DeltaTime);
    void HandleStaringState(float DeltaTime);
    void HandleAggressiveState(float DeltaTime);
    void HandleRetreatingState(float DeltaTime);
    void SetCurrentBehaviorState(EEyeMonsterState NewState);

    // === STARING SYSTEM ===

    void ProcessStaring(float DeltaTime);
    void OnPlayerStartedLooking();
    void OnPlayerStoppedLooking();
    void ApplyStaringSanityDamage(float DeltaTime);
    void ApplyAggressiveSanityDamage(float DeltaTime);
    void ResetStareData();

    // === PROXIMITY FEAR ===

    void CheckProximityFear(float DeltaTime);

    // === TELEPORTATION SYSTEM ===

    void PerformStealthBlink();
    void PerformAggressiveBlink();
    void PerformEscapeBlink();
    void PerformRetreatTeleport();

    // Position Finding
    FVector FindStealthyPosition();
    FVector FindAggressivePosition(const FVector& PlayerLocation);
    FVector FindEscapePosition();
    FVector FindRetreatPosition();

    // Teleport Conditions
    bool CanPerformBlink() const;
    bool ShouldRandomBlink() const;

    // === AGGRESSIVE STATE ===

    void TriggerAggressiveResponse();
    void TriggerAggressiveEffects();
    void ShowIntenseBloodOverlay();
    void SpawnAggressiveEffects();

    // === VISUAL EFFECTS ===

    void StartEyeGlow();
    void StopEyeGlow();
    void UpdateVisualEffects(float DeltaTime);
    void TriggerSanityDamageEffects(float DamageAmount);
    void TriggerProximityFearEffect();
    void TriggerBlinkEffects();

    // === AUDIO SYSTEM ===

    void PlayStareSound();
    void PlayAggressiveSound();
    void PlayStealthSound();
    void PlayEscapeSound();
    void PlayRetreatSound();
    void PlayDamageSound() const;

    // === UTILITY METHODS ===

    bool IsPlayerLookingAtMonster() const;
    FVector GetPlayerLocation() const;
    void FacePlayer(float DeltaTime);

    // Legacy UI Methods
    void ShowBloodOverlay(APlayerController* PC, float Intensity = 1.0f);
    void RemoveBloodOverlay();
    void ShakeCamera(APlayerController* PC, float Intensity = 1.0f) const;

    // === SPAWNING SYSTEM ===

    void DrawSpawnDebug() const;
    void RespawnAndDestroy();
    void HandleSelfDestruct();
    void SpawnAtRandomLocation();
    bool GetGroundSpawnLocation(const FVector2D& XY, FVector& OutLocation) const;

    // Legacy Blink Methods
    void FinishBlink();

private:
    // === CACHED COMPONENTS ===

    UPROPERTY()
    USanityComponent* CachedSanityComponent;

    UPROPERTY()
    UUserWidget* BloodOverlayWidget;

    // === STATE VARIABLES ===

    // Staring State
    bool bIsStaring = false;
    float CurrentStareTime = 0.0f;
    float CurrentStareIntensity = 1.0f;
    float SanityDamageTimer = 0.0f;

    // Aggressive State
    float AggressiveCooldownTimer = 0.0f;
    float AggressiveDamageTimer = 0.0f;

    // Retreating State
    float RetreatCooldownTimer = 0.0f;

    // Proximity Fear
    float ProximityFearTimer = 0.0f;

    // Player Awareness
    bool bPlayerAware = false;

    // Teleportation
    float LastBlinkTime = 0.0f;

    // Visual Effects
    bool bEyesGlowing = false;
    float EyeGlowIntensity = 1.0f;

    // Legacy Animation State
    bool bIsBlinking = false;
    bool bIsCoveringEyes = false;

    // === SPAWN SYSTEM ===

    FVector InitialSpawnCenter;
    FVector InitialSpawnExtent;

    // === TIMER HANDLES ===

    FTimerHandle DestroyTimerHandle;
    FTimerHandle RespawnTimerHandle;
    FTimerHandle BloodOverlayTimerHandle;
    FTimerHandle BlinkTimerHandle;
    FTimerHandle BlinkEffectTimer;

    // === HELPER FUNCTIONS ===

    bool CanDamage() const { return CachedSanityComponent != nullptr; }
};