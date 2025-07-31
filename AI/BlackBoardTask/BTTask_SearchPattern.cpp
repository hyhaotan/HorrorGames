// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SearchPattern.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "HorrorGame/AI/NPC.h"

UBTTask_SearchPattern::UBTTask_SearchPattern()
{
    NodeName = TEXT("Search Pattern");
}

EBTNodeResult::Type UBTTask_SearchPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    ANPC* NPC = Cast<ANPC>(AICon->GetPawn());
    if (!NPC) return EBTNodeResult::Failed;

    // Lấy điểm tìm kiếm tiếp theo
    FVector NextSearchLocation = NPC->GetNextSearchLocation();

    // Gán vào Blackboard (key là "SearchLocation")
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(TEXT("SearchLocation"), NextSearchLocation);
    }

    // Tăng chỉ số tìm kiếm nếu cần
    NPC->CurrentSearchIndex++;

    // Nếu đã hết điểm tìm kiếm, có thể trả về Succeeded và dừng search
    if (NPC->CurrentSearchIndex >= NPC->MaxSearchPoints)
    {
        NPC->StopSearchPattern();
        if (BB) BB->SetValueAsBool(TEXT("IsSearching"), false);
        return EBTNodeResult::Succeeded;
    }

    // Nếu còn điểm, trả về InProgress để tiếp tục
    return EBTNodeResult::InProgress;
}

