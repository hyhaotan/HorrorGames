#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/Engine.h"
#include "SanityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSanityChanged, float, NewSanityPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSanityThresholdReached, float, ThresholdPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPanicAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHallucination);

UENUM(BlueprintType)
enum class ESanityState : uint8
{
    Stable      UMETA(DisplayName = "Stable (80-100%)"),
    Nervous     UMETA(DisplayName = "Nervous (60-80%)"),
    Anxious     UMETA(DisplayName = "Anxious (40-60%)"),
    Unstable    UMETA(DisplayName = "Unstable (20-40%)"),
    Critical    UMETA(DisplayName = "Critical (0-20%)")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API USanityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USanityComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void RecoverSanity(float Delta, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void DrainSanity(float Delta, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void SetSanity(float NewSanity);

    UFUNCTION(BlueprintPure, Category = "Sanity")
    float GetSanity() const { return CurrentSanity; }

    UFUNCTION(BlueprintPure, Category = "Sanity")
    float GetSanityPercent() const { return CurrentSanity / MaxSanity; }

    UFUNCTION(BlueprintPure, Category = "Sanity")
    float GetMaxSanity() const { return MaxSanity; }

    UFUNCTION(BlueprintPure, Category = "Sanity")
    ESanityState GetSanityState() const;

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void StartEnvironmentalDrain();

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void StopEnvironmentalDrain();

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void PauseSanitySystem();

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void ResumeSanitySystem();

    // Event triggers - Điều chỉnh giá trị mặc định hợp lý hơn
    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void OnMonsterSighting(float SanityLoss = 15.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void OnMonsterProximity(float SanityLoss = 8.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void OnSupernaturalEvent(float SanityLoss = 12.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void OnJumpScare(float SanityLoss = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void OnCorpseDiscovery(float SanityLoss = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void OnSeeGhost(float SanityLoss = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void OnDarknessExposure(float SanityLoss = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void OnSafeZoneEnter();

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void OnLightSource(float RecoveryAmount = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void TriggerPanicAttack();

    UFUNCTION(BlueprintCallable, Category = "Sanity")
    void TriggerHallucination();

    void TriggerLowSanityPostProcess();

    // Delegates
    UPROPERTY(BlueprintAssignable)
    FOnSanityChanged OnSanityChanged;

    UPROPERTY(BlueprintAssignable)
    FOnSanityThresholdReached OnSanityThresholdReached;

    UPROPERTY(BlueprintAssignable)
    FOnPanicAttack OnPanicAttack;

    UPROPERTY(BlueprintAssignable)
    FOnHallucination OnHallucination;

    bool bInPanicAttack;

protected:
    // Sanity Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Settings", meta = (ClampMin = "50.0", ClampMax = "200.0"))
    float MaxSanity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Settings", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float StartingSanity = 100.0f;

    // Drain/Recovery Rates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Rates", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float EnvironmentalDrainRate = 0.5f; // Rất chậm trong môi trường bình thường

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Rates", meta = (ClampMin = "0.1", ClampMax = "20.0"))
    float DarknessDrainMultiplier = 3.0f; // Tăng drain khi ở trong bóng tối

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Rates", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float SafeZoneRecoveryRate = 2.0f; // Hồi phục trong safe zone

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Rates", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float NaturalRecoveryRate = 0.1f; // Hồi phục tự nhiên rất chậm

    // Thresholds for different sanity levels (cập nhật để phù hợp với enum)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Thresholds")
    TArray<float> SanityThresholds = { 0.8f, 0.6f, 0.4f, 0.2f };

    // Timeline cho các hiệu ứng
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Timeline")
    class UCurveFloat* SanityDrainCurve;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity|Timeline")
    class UTimelineComponent* SanityTimeline;

    // Post Process Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Effects")
    TSubclassOf<class UCameraShakeBase> LowSanityShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Effects")
    TSubclassOf<class UCameraShakeBase> PanicShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Effects")
    float LowSanityEffectThreshold = 0.6f;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Audio")
    class USoundBase* HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Audio")
    class USoundBase* WhisperSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Audio")
    class USoundBase* PanicSound;

    // Widget
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|UI")
    TSubclassOf<class USanityWidget> SanityWidgetClass;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Performance", meta = (ClampMin = "0.05", ClampMax = "1.0"))
    float UpdateFrequency = 0.1f;

    // Panic Attack Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Panic")
    float PanicAttackChance = 0.05f; // 5% chance mỗi giây khi sanity < 20%

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Panic")
    float PanicAttackDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Panic")
    float PanicAttackCooldown = 15.0f;

    // Hallucination Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Hallucination")
    float HallucinationChance = 0.02f; // 2% chance mỗi giây khi sanity < 40%

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Hallucination")
    float HallucinationCooldown = 30.0f;

    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Debug")
    bool bDebugSanity = false;

private:
    // State
    float CurrentSanity;
    float LastUpdateTime;
    bool bEnvironmentalDrainActive;
    bool bInSafeZone;
    bool bSystemPaused;
    ESanityState CurrentState;
    ESanityState PreviousState;
    FTimerHandle FadeTimerHandle;

    // Panic/Hallucination tracking
    float LastPanicTime;
    float LastHallucinationTime;

    // Threshold tracking
    TArray<bool> ThresholdTriggered;

    // Sanity modifiers (for temporary effects)
    UPROPERTY()
    TMap<FString, float> SanityModifiers;

    // Components
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class APlayerController* PlayerController;

    UPROPERTY()
    class UPostProcessComponent* PostProcessComponent;

    UPROPERTY()
    class USanityWidget* SanityWidget;

    UPROPERTY()
    class UHeadbobComponent* HeadbobComponent;

    UPROPERTY()
    class UAudioComponent* HeartbeatAudioComponent;

    // Internal functions
    void InitializeSanityTimeline();
    void SetupSanityWidget();
    void UpdatePostProcessEffects();
    void UpdateAudioEffects();
    void CheckSanityThresholds();
    void CheckSanityState();
    void CheckRandomEvents();
    void NotifyHeadbobComponent();
    void ProcessSanityChange(float Delta, bool bInstant);

    UFUNCTION()
    void HandleEnvironmentalDrain(float Value);

    // Helper functions
    float GetCurrentDrainRate() const;
    float GetPostProcessIntensity() const;
    float GetTotalSanityModifier() const;
    bool ValidateComponents();
    bool CanTriggerPanicAttack() const;
    bool CanTriggerHallucination() const;

	void OnSafeZoneExit();
    void AddSanityModifier(const FString& ModifierName, float ModifierValue);
    void RemoveSanityModifier(const FString& ModifierName);
    void ResetSanityToMax();
    void SetMaxSanity(float NewMaxSanity);
    FString GetSanityStateString() const;
    void DebugPrintSanityInfo() const;
};