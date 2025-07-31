// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_BeginSearchPattern.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "HorrorGame/AI/NPC.h"

UBTTask_BeginSearchPattern::UBTTask_BeginSearchPattern()
{
    NodeName = TEXT("Begin Search Pattern");
}

EBTNodeResult::Type UBTTask_BeginSearchPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    ANPC* NPC = Cast<ANPC>(AICon->GetPawn());
    if (!NPC) return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    // L?y v? trí cu?i cùng th?y player t? blackboard
    FVector LastKnownLocation = BB->GetValueAsVector(TEXT("LastKnownPlayerLocation"));

    // G?i StartSearchPattern và set IsSearching
    NPC->StartSearchPattern(LastKnownLocation);
    NPC->SetIsSearching(true);
    BB->SetValueAsBool(TEXT("IsSearching"), true);

    return EBTNodeResult::Succeeded;
}

