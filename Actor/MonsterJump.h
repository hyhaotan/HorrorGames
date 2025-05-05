#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "MonsterJump.generated.h"

UENUM()
enum class EQTEResult { Perfect, Good, Miss };

UCLASS()
class HORRORGAME_API AMonsterJump : public ACharacter
{
    GENERATED_BODY()

public:
    AMonsterJump();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // gọi khi người chơi nhấn phím escape
    void ReceiveEscapeInput(FKey PressedKey);

protected:
    virtual void BeginPlay() override;

    // Khởi tạo QTE (chuỗi đầu tiên)
    void StartQTE(bool bClearProgress);
    // Sinh lại chuỗi các phím nhưng không reset progress
    void NextQTESequence();
    void CompleteEscape();
    void UpdateWidget();
    void ApplyStun(float Duration);
    void ReleaseStun();
    void BonusIncrementPerStep(float AmountPer);
    void AdjustDifficulty();
    EQTEResult EvaluateTiming(float DeltaFromTarget);

    // Trigger zone
    UPROPERTY(VisibleAnywhere)
    UBoxComponent* TriggerZone;

    UPROPERTY(EditAnywhere, Category = "Stun")
	TSubclassOf <class UCameraShakeBase>  StunCameraShake;

    // QTE sequence
    TArray<FKey> QTESequence;
    int32 CurrentQTEIndex = 0;
    int32 SequenceLength = 4;

    // progress
    float EscapeProgress;
    float EscapeTarget = 1.0f;
    float IncrementPerStep = 0.05f;
    float AllowedInputTime = 1.0f;
    float PerfectThreshold;
    float GoodThreshold;
    float LastPromptTime;

    int32 TotalHits = 0;
    int32 TotalMisses = 0;
    int32 MissCount = 0;
	int32 ComboCount = 0;
    bool bIsStunned = false;

    bool bIsGrabbing;
    class AHorrorGameCharacter* CapturedPlayer;

	UPROPERTY(EditAnywhere, Category = "Stun")
	class UAnimMontage* StunMontage;      
    
    UPROPERTY(EditAnywhere, Category = "Stun")
	class UAnimMontage* StunReverseMontage;

    UPROPERTY(EditAnywhere, Category = "Stun")
	class USoundBase* StunSound;

    // UI
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<class UProgressBarWidget> ProgressBarClass;
    UProgressBarWidget* EscapeWidget;

    FTimerHandle StunTimerHandle;
};
