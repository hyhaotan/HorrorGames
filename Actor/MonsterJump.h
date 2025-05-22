#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Engine/DataTable.h"
#include "HorrorGame/Data/QTEIconRow.h"
#include "MonsterJump.generated.h"

class AHorrorGameCharacter;
class UDataTable;
class UTexture2D;
class UProgressBarWidget;

UENUM(BlueprintType)
enum class EQTEPhase : uint8
{
    WASD        UMETA(DisplayName = "WASD"),
    Arrows      UMETA(DisplayName = "Arrows"),
};

UENUM(BlueprintType)
enum class EQTEResult : uint8
{
    Perfect UMETA(DisplayName = "Perfect"),
    Good    UMETA(DisplayName = "Good"),
    Miss    UMETA(DisplayName = "Miss")
};

UCLASS()
class HORRORGAME_API AMonsterJump : public ACharacter
{
    GENERATED_BODY()

public:
    AMonsterJump();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void ReceiveEscapeInput(FKey PressedKey);

    EQTEPhase GetCurrentPhase() const { return CurrentPhase; }

protected:
    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    void StartQTE(bool bClearProgress);
    void NextQTESequence();
    EQTEResult EvaluateTiming(float DeltaFromTarget) const;
    void UpdateWidget();
    void CompleteEscape();
    void AdjustDifficulty();
    void ChooseRandomPhase();
    void GenerateSequenceByPhase();

    void ApplyStun(float Duration);
    void ReleaseStun();

    void InitializeGrabbedPlayer(AHorrorGameCharacter* Player);

    UPROPERTY(VisibleAnywhere, Category = "Trigger")
    UBoxComponent* TriggerZone;

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

    UPROPERTY(EditAnywhere, Category = "Stun|Animation")
    UAnimMontage* StunMontage;

    UPROPERTY(EditAnywhere, Category = "Stun|Animation")
    UAnimMontage* StunReverseMontage;

    UPROPERTY(EditAnywhere, Category = "Stun|Effects")
    TSubclassOf<UCameraShakeBase> StunCameraShake;

    UPROPERTY(EditAnywhere, Category = "Stun|Effects")
    USoundBase* StunSound;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UProgressBarWidget> ProgressBarClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QTE")
    EQTEPhase CurrentPhase;

    //UPROPERTY(EditAnywhere, Category = "QTE|Data")
    //UDataTable* KeyIconTable;

    //UPROPERTY(EditAnywhere, Category = "QTE|Data")
    //UDataTable* PhaseIconTable;

    /** Map key → icon texture */
    UPROPERTY(EditAnywhere, Category = "QTE")
    TMap<FName, TSoftObjectPtr<UTexture2D>> KeyIcons;

    /** Map phase → icon texture */
    UPROPERTY(EditAnywhere, Category = "QTE")
    TMap<EQTEPhase, TSoftObjectPtr<UTexture2D>> PhaseIcons;

private:
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

    AHorrorGameCharacter* CapturedPlayer = nullptr;
    FTimerHandle StunTimerHandle;
    UProgressBarWidget* EscapeWidget = nullptr;

    UTexture2D* GetKeyIconTexture(const FKey& Key) const;
    UTexture2D* GetPhaseIconTexture(EQTEPhase Phase) const;
};