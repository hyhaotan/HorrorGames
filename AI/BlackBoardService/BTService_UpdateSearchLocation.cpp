// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_UpdateSearchLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "HorrorGame/AI/NPC.h"

UBTService_UpdateSearchLocation::UBTService_UpdateSearchLocation()
{
    NodeName = TEXT("Update Search Location");
}

void UBTService_UpdateSearchLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return;

    ANPC* NPC = Cast<ANPC>(AICon->GetPawn());
    if (!NPC) return;

    // Lấy điểm tìm kiếm tiếp theo
    FVector NextSearchLocation = NPC->GetNextSearchLocation();

    // Gán vào Blackboard (key là "SearchLocation")
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(TEXT("SearchLocation"), NextSearchLocation);
    }
}

