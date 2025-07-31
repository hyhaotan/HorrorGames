#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Engine/TimerHandle.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimMontage.h"
#include "BTTask_JumpScare.generated.h"

UCLASS()
class HORRORGAME_API UBTTask_JumpScare : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_JumpScare();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type Result) override;

protected:
    // Animation Montage cho jump scare
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpScare")
    UAnimMontage* JumpScareMontage;

    // Sound effect cho jump scare
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpScare")
    USoundCue* JumpscareSound;

    // Thời gian jump scare kéo dài
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpScare", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float JumpScareDuration = 3.0f;

    // Thời gian blend camera
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpScare", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float BlendTime = 0.5f;

    // Khoảng cách tối đa để jumpscare có hiệu lực
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpScare", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float MaxJumpscareDistance = 1000.0f;

private:
    // Timer handle
    FTimerHandle TimerHandle_JumpScare;

    // Cached references
    UPROPERTY()
    UBehaviorTreeComponent* CachedOwnerComp;

    // Flag để tránh play sound nhiều lần
    bool bHasPlayedJumpScareSound = false;

    // Callback functions
    void OnJumpScareComplete();

    // Helper functions
    void ResetBlackboardKeys();
    void RestartAILogic();

    // Multiplayer helper - tìm tất cả players trong khu vực
    TArray<class AHorrorGameCharacter*> GetNearbyPlayers(const FVector& NPCLocation) const;
};