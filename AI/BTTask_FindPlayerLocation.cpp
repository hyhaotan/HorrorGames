// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/AI/BTTask_FindPlayerLocation.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindPlayerLocation::UBTTask_FindPlayerLocation(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = TEXT("Find Player Location");
}

EBTNodeResult::Type UBTTask_FindPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//l?y nhân v?t ng??i ch?i
	if (auto* const Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		// l?y v? trí c?a ng??i ch?i ?? s? d?ng nó làm ngu?n g?c
		auto const PlayerLocation = Player->GetActorLocation();
		if (SearchRandom)
		{
			FNavLocation Loc;

			//l?y h? th?ng ??nh v? và t?o m?t v? trí ng?u nhiên g?n ng??i ch?i
			if (auto* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
			{
				//th? l?y 1 v? trí ng?u nhiên c?a ng??i ch?i
				if (NavSys->GetRandomPointInNavigableRadius(PlayerLocation, SeachRadius, Loc))
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), Loc.Location);
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
					return EBTNodeResult::Succeeded;
				}
			}
		}
		else
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), PlayerLocation);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}