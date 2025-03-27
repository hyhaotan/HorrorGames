// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_JumpScare.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API UBTTask_JumpScare : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
    UBTTask_JumpScare();

    // Override hàm ExecuteTask để thực hiện logic task
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // Override hàm OnTaskFinished để dọn dẹp timer nếu cần
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type Result) override;

protected:
    // Callback được gọi khi JumpScareDuration kết thúc
    void OnJumpScareComplete();

    // Timer handle để quản lý delay của jump scare
    FTimerHandle TimerHandle_JumpScare;

    // Cached pointer để gọi FinishLatentTask khi task hoàn thành
    UBehaviorTreeComponent* CachedOwnerComp;

    // Thời gian chờ jump scare (sau khi hết thời gian này sẽ chuyển lại camera về người chơi)
    UPROPERTY(EditAnywhere, Category = "JumpScare")
    float JumpScareDuration = 3.0f;

    // Thời gian blend khi chuyển camera (chuyển sang và quay lại)
    UPROPERTY(EditAnywhere, Category = "JumpScare")
    float BlendTime = 0.5f;

    UPROPERTY(EditAnywhere,Category="Jumpscare Sound")
    class USoundBase* JumpscareSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpScare")
    bool bHasPlayedJumpScareSound = false;

    // Animation Montage được phát trên NPC khi jump scare diễn ra
    UPROPERTY(EditAnywhere, Category = "JumpScare")
    UAnimMontage* JumpScareMontage;
};
