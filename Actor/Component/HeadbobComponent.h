#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"
#include "HeadbobComponent.generated.h"

UENUM(BlueprintType)
enum class EHeadbobState : uint8
{
    None,
    Walking,
    Sprinting,
    Crouching,
    Idle,
    Custom
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeadbobStateChanged, EHeadbobState, NewState, EHeadbobState, OldState);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API UHeadbobComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHeadbobComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void StartHeadbob();

    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void StopHeadbob();

    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void ForceUpdateHeadbob();

    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void SetHeadbobEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Headbob")
    bool IsHeadbobActive() const { return bIsCurrentlyHeadbobbing; }

    UFUNCTION(BlueprintPure, Category = "Headbob")
    EHeadbobState GetCurrentHeadbobState() const { return CurrentHeadbobState; }

    UFUNCTION(BlueprintCallable, Category = "Headbob")
    void SetCustomHeadbob(TSubclassOf<UCameraShakeBase> CustomShake, float Intensity);

    // Delegates
    UPROPERTY(BlueprintAssignable)
    FOnHeadbobStateChanged OnHeadbobStateChanged;

protected:
    virtual void BeginPlay() override;

    // Camera Shake Classes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Shakes")
    TSubclassOf<UCameraShakeBase> WalkCameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Shakes")
    TSubclassOf<UCameraShakeBase> SprintCameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Shakes")
    TSubclassOf<UCameraShakeBase> CrouchCameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Shakes")
    TSubclassOf<UCameraShakeBase> IdleCameraShakeClass;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Performance", meta = (ClampMin = "0.05", ClampMax = "0.5"))
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Performance", meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float MinSpeedForHeadbob = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Performance", meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float SpeedChangeThreshold = 75.0f; // How much speed must change to trigger shake update

    // Intensity Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float WalkIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float SprintIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float CrouchIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity", meta = (ClampMin = "0.2", ClampMax = "1.0"))
    float MinIntensityMultiplier = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Intensity", meta = (ClampMin = "1.0", ClampMax = "3.0"))
    float MaxIntensityMultiplier = 1.5f;

    // Transition Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Transitions")
    bool bSmoothTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Transitions", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float TransitionFadeTime = 0.3f;

    // Debug Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Headbob|Debug")
    bool bDebugHeadbob = false;

private:
    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class APlayerController* PlayerController;

    UPROPERTY()
    class USprintComponent* SprintComponent;

    // State tracking
    EHeadbobState CurrentHeadbobState;
    EHeadbobState PreviousHeadbobState;
    bool bIsCurrentlyHeadbobbing;
    bool bHeadbobEnabled;

    // Current shake info
    UPROPERTY()
    class UCameraShakeBase* CurrentCameraShake;
    TSubclassOf<UCameraShakeBase> CurrentShakeClass;
    float CurrentIntensity;

    // Timing
    float LastUpdateTime;
    float LastSpeedCheck;
    float LastMovementSpeed;

    // Custom headbob
    TSubclassOf<UCameraShakeBase> CustomShakeClass;
    float CustomIntensity;

    // Core functions
    void UpdateHeadbobState();
    void ProcessHeadbobTransition();
    void ApplyHeadbob();
    void ClearCurrentShake();

    // Helper functions
    EHeadbobState DetermineHeadbobState() const;
    bool ShouldPlayHeadbob() const;
    float CalculateIntensity() const;
    TSubclassOf<UCameraShakeBase> GetShakeClassForState(EHeadbobState State) const;
    float GetBaseIntensityForState(EHeadbobState State) const;

    // Validation
    bool ValidateComponents();
    void LogHeadbobChange(EHeadbobState NewState, float Speed, float Intensity) const;
};