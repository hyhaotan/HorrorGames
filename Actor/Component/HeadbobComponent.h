#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraShakeBase.h"
#include "HeadbobComponent.generated.h"

UENUM(BlueprintType)
enum class EHeadbobState : uint8
{
    None        UMETA(DisplayName = "None"),
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Custom      UMETA(DisplayName = "Custom")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeadbobStateChanged, EHeadbobState, NewState, EHeadbobState, PreviousState);

class USprintComponent;
class USanityComponent;
class ACharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API UHeadbobComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHeadbobComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void StartHeadbob();

    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void StopHeadbob();

    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void ForceUpdateHeadbob();

    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void SetHeadbobEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void SetCustomHeadbob(TSubclassOf<UCameraShakeBase> CustomShake, float Intensity);

    // Returns a vector offset (local) to apply to camera/neck
    UFUNCTION(BlueprintCallable, Category = "Headbob")
    FVector GetHeadbobOffset() const;

    // Delegate
    UPROPERTY(BlueprintAssignable, Category = "Headbob")
    FOnHeadbobStateChanged OnHeadbobStateChanged;

protected:
    // Core logic
    void UpdateHeadbobState();
    void ProcessHeadbobTransition();
    void ApplyHeadbob();
    void ClearCurrentShake();

    EHeadbobState DetermineHeadbobState() const;
    bool ShouldPlayHeadbob() const;
    float CalculateIntensity() const;
    TSubclassOf<UCameraShakeBase> GetShakeClassForState(EHeadbobState State) const;
    float GetBaseIntensityForState(EHeadbobState State) const;

    // NEW: base headbob waveform generator used by GetHeadbobOffset
    FVector CalculateBaseHeadbob() const;

    // Sanity influence
    float GetSanityModifier() const;
    void UpdateHeadbobIntensity();

    void LogHeadbobChange(EHeadbobState NewState, float Speed, float Intensity) const;
    bool ValidateComponents();

protected:
    // Owner refs
    UPROPERTY(Transient)
    ACharacter* OwnerCharacter;

    UPROPERTY(Transient)
    APlayerController* PlayerController;

    UPROPERTY(Transient)
    USprintComponent* SprintComponent;

    UPROPERTY(Transient)
    USanityComponent* SanityComponent;

    // State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Headbob")
    EHeadbobState CurrentHeadbobState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Headbob")
    EHeadbobState PreviousHeadbobState;

    bool bIsCurrentlyHeadbobbing;

    // Controls whether camera shake (PlayerCameraManager) is toggled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob")
    bool bHeadbobEnabled;

    // Controls whether GetHeadbobOffset() produces non-zero offsets (useful to disable visual offset while keeping camera shakes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob")
    bool bEnableHeadbob;

    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Debug")
    bool bDebugHeadbob;

    // Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob")
    float UpdateFrequency = 0.05f;

    float LastUpdateTime;

    // Camera shake management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|CameraShake")
    TSubclassOf<UCameraShakeBase> WalkCameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|CameraShake")
    TSubclassOf<UCameraShakeBase> SprintCameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|CameraShake")
    TSubclassOf<UCameraShakeBase> CrouchCameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|CameraShake")
    TSubclassOf<UCameraShakeBase> IdleCameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|CameraShake")
    TSubclassOf<UCameraShakeBase> CustomShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|CameraShake")
    bool bSmoothTransitions = true;

    // Intensities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity")
    float WalkIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity")
    float SprintIntensity = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity")
    float CrouchIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity")
    float IdleIntensity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity")
    float BaseHeadbobIntensity = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Headbob")
    float CurrentIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob")
    float CustomIntensity = 1.0f;

    // Speed thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob")
    float MinSpeedForHeadbob = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity")
    float MinIntensityMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity")
    float MaxIntensityMultiplier = 2.5f;

    // Camera shake runtime
    UPROPERTY(Transient)
    UCameraShakeBase* CurrentCameraShake;

    UPROPERTY(Transient)
    TSubclassOf<UCameraShakeBase> CurrentShakeClass;

    // other runtime
    float LastMovementSpeed;

    // Optional: tuning for CalculateBaseHeadbob
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Tuning")
    float MaxHeadbobOffset = 1.5f; // in cm or world-units, tune to taste
};