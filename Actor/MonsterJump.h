
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "MonsterJump.generated.h"

class UProgressBarWidget;

UCLASS()
class HORRORGAME_API AMonsterJump : public ACharacter
{
    GENERATED_BODY()

public:
    AMonsterJump();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Trigger volume to detect player overlap
    UPROPERTY(VisibleAnywhere, Category = "Trigger")
    UBoxComponent* TriggerZone;

    // Progress bar widget class
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UProgressBarWidget> ProgressBarClass;

    // Instance of the escape widget
    UPROPERTY()
    UProgressBarWidget* EscapeWidget;

    // Escape progress tracking
    float EscapeProgress;
    UPROPERTY(EditAnywhere, Category = "Escape")
    float IncrementPerPress = 0.1f;
    UPROPERTY(EditAnywhere, Category = "Escape")
    float EscapeTarget = 1.0f;

    bool bIsGrabbing;

    // Overlap callback
    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

public:
    // Called by character when player presses Escape
    void ReceiveEscapeInput();

    UPROPERTY()
    class AHorrorGameCharacter* CapturedPlayer;
};