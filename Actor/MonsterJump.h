
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "MonsterJump.generated.h"

// QTE result enumeration
UENUM(BlueprintType)
enum class EQTEResult : uint8
{
    Perfect UMETA(DisplayName = "Perfect"),
    Good    UMETA(DisplayName = "Good"),
    Miss    UMETA(DisplayName = "Miss")
};

UENUM(BlueprintType)
enum class EQTEPhase : uint8
{
    WASD        UMETA(DisplayName = "WASD"),
    Arrows      UMETA(DisplayName = "Arrows"),
    Opposite    UMETA(DisplayName = "OppositePairs")
};

class AHorrorGameCharacter;

UCLASS()
class HORRORGAME_API AMonsterJump : public ACharacter
{
    GENERATED_BODY()

public:
    // Constructor & overrides
    AMonsterJump();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Input handling
    /** Called when the player presses the escape key during QTE */
    void ReceiveEscapeInput(FKey PressedKey);

    FKey GetOppositeKey1() const { return OppositeKey1; }
    FKey GetOppositeKey2() const { return OppositeKey2; }
    EQTEPhase GetCurrentPhase() const { return CurrentPhase; }

protected:
    // --- Overlap / Trigger ---
    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    // --- QTE workflow ---
    /** Initialize or reset QTE sequence */
    void StartQTE(bool bClearProgress);

    /** Generate next key in sequence without resetting progress */
    void NextQTESequence();

    /** Evaluate input timing against thresholds */
    EQTEResult EvaluateTiming(float DeltaFromTarget) const;

    /** Update on-screen QTE widget */
    void UpdateWidget();

    /** Handle QTE completion */
    void CompleteEscape();

    /** Adjust difficulty dynamically based on performance */
    void AdjustDifficulty();

    // hàm chọn random phase
    void ChooseRandomPhase();
    // hàm sinh sequence theo phase
    void GenerateSequenceByPhase();

    // --- Stun effects ---
    /** Apply stun effect for duration */
    void ApplyStun(float Duration);
    /** Release stun effect */
    void ReleaseStun();

	void InitializeGrabbedPlayer(AHorrorGameCharacter* Player);

    // --- Properties ---

    // Trigger zone for grabbing player
    UPROPERTY(VisibleAnywhere, Category = "Trigger")
    UBoxComponent* TriggerZone;

    // QTE configuration
    UPROPERTY(EditAnywhere, Category = "QTE")
    int32 SequenceLength = 4;

    UPROPERTY(EditAnywhere, Category = "QTE|Timing")
    float AllowedInputTime = 1.0f;   

    UPROPERTY(EditAnywhere, Category = "QTE|Timing")
    float PerfectThreshold;             

    UPROPERTY(EditAnywhere, Category = "QTE|Timing")
    float GoodThreshold;

    UPROPERTY(VisibleInstanceOnly, Category = "QTE|Progress")
    float EscapeProgress = 0.0f;

    UPROPERTY(EditAnywhere, Category = "QTE|Progress")
    float EscapeTarget = 1.0f;

    UPROPERTY(EditAnywhere, Category = "QTE|Progress")
    float IncrementPerStep = 0.05f;

    // Sound & animation for stun
    UPROPERTY(EditAnywhere, Category = "Stun|Animation")
    UAnimMontage* StunMontage;

    UPROPERTY(EditAnywhere, Category = "Stun|Animation")
    UAnimMontage* StunReverseMontage;

    UPROPERTY(EditAnywhere, Category = "Stun|Effects")
    TSubclassOf<UCameraShakeBase> StunCameraShake;

    UPROPERTY(EditAnywhere, Category = "Stun|Effects")
    USoundBase* StunSound;

    // UI widget for escape progress
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UProgressBarWidget> ProgressBarClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QTE")
    EQTEPhase CurrentPhase;

private:
    // Runtime state
    TArray<FKey> QTESequence;
    int32 CurrentQTEIndex = 0;
    float LastPromptTime = 0.0f;

    int32 TotalHits = 0;
    int32 TotalMisses = 0;
    int32 MissCount = 0;
    int32 ComboCount = 0;
    bool bIsStunned = false;
    bool bIsGrabbing = false;
    bool bPhaseInitialized = false;

    FKey OppositeKey1;
    FKey OppositeKey2;

    // Reference to captured player during QTE
    AHorrorGameCharacter* CapturedPlayer = nullptr;

    // Handle for stun timer
    FTimerHandle StunTimerHandle;

    // Runtime widget instance
    UProgressBarWidget* EscapeWidget = nullptr;
};
