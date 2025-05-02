#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "MonsterJump.generated.h"

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

    // Trigger zone
    UPROPERTY(VisibleAnywhere)
    UBoxComponent* TriggerZone;

    // QTE sequence
    TArray<FKey> QTESequence;
    int32 CurrentQTEIndex = 0;
    int32 SequenceLength = 4;

    // progress
    float EscapeProgress;
    float EscapeTarget = 1.0f;
    float IncrementPerStep = 0.1f;

    bool bIsGrabbing;
    class AHorrorGameCharacter* CapturedPlayer;

    // UI
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<class UProgressBarWidget> ProgressBarClass;
    UProgressBarWidget* EscapeWidget;
};
