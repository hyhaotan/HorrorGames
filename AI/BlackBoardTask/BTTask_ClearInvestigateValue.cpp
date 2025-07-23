// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ClearInvestigateValue.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HorrorGame/AI/NPC.h"
#include "HorrorGame/AI/NPC_AIController.h"

UBTTask_ClearInvestigateValue::UBTTask_ClearInvestigateValue(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Clear Investigate Value");
}

EBTNodeResult::Type UBTTask_ClearInvestigateValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Xóa giá trị trên Blackboard cho key "Investigation Pos"
	if (OwnerComp.GetBlackboardComponent())
	{
		OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
	
		// Cast từ Controlled Pawn sang ANPC
		if (AAIController* AIController = OwnerComp.GetAIOwner())
		{
			ANPC* MyNPC = Cast<ANPC>(AIController->GetPawn());
			if (MyNPC)
			{
				// Thực hiện hành động cần thiết với MyNPC, ví dụ:
				MyNPC->ToggleInvestigationWidgetVisibility();
			}
		}
	}

	// Trả về Success để hoàn thành nhiệm vụ
	return EBTNodeResult::Succeeded;
}