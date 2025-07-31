// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SearchPattern.generated.h"

/**
 * Task cho AI thực hiện tìm kiếm theo Search Pattern.
 */
UCLASS()
class HORRORGAME_API UBTTask_SearchPattern : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTTask_SearchPattern();

protected:
    // Hàm thực thi task
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
