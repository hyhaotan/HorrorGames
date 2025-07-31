// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_UpdateSearchLocation.generated.h"

UCLASS()
class HORRORGAME_API UBTService_UpdateSearchLocation : public UBTService_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTService_UpdateSearchLocation();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
