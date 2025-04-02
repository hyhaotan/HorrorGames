// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/AI/BTTask_FindPathPoint.h"
#include "NPC_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPC.h"

UBTTask_FindPathPoint::UBTTask_FindPathPoint(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = TEXT("Find Path Point");
}

EBTNodeResult::Type UBTTask_FindPathPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//L?y b? ?i?u khi?n c?a NPC 
	if (auto* const cont = Cast<ANPC_AIController>(OwnerComp.GetAIOwner()))
	{
		//l?y th�nh ph?n c?a blackbroad component from behavior tree
		if (auto* const bc = OwnerComp.GetBlackboardComponent())
		{
			//l?y ch? s? ???ng tu?n tra hi?n t?i t? the blackbroad
			auto const Index = bc->GetValueAsInt((GetSelectedBlackboardKey()));

			//l?y NPC
			if (auto* npc = Cast<ANPC>(cont->GetPawn()))
			{
				//l?y vector ???ng tu?n tra hi?n t?i t? npc - ?�y l� ??a ph??ng c?a di?n vi�n ???ng tu?n tra
				auto const Point = npc->GetPatrolPath()->GetPatrolPoint(Index);

				//chuy?n ??i vector c?c b? th�nh ?i?m to�n c?u
				auto const GlobalPoint = npc->GetPatrolPath()->GetActorTransform().TransformPosition(Point);
				bc->SetValueAsVector(PatrolPathVectorKey.SelectedKeyName, GlobalPoint);

				//k?t th�c khi th�nh c�ng
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}