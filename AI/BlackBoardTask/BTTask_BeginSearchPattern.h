// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BeginSearchPattern.generated.h"

/**
 * Task to begin search pattern: set IsSearching, set LastKnownPlayerLocation, call StartSearchPattern
 */
UCLASS()
class HORRORGAME_API UBTTask_BeginSearchPattern : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UBTTask_BeginSearchPattern();
protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
