// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/AI/BTService_IsPlayerInMeleeRange.h"
#include "NPC_AIController.h"
#include "NPC.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_IsPlayerInMeleeRange::UBTService_IsPlayerInMeleeRange()
{
	bNotifyBecomeRelevant = true;
	NodeName = TEXT("Is Player In Melee Range");
}

void UBTService_IsPlayerInMeleeRange::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//get Controller and NPC
	auto const* const Cont = Cast<ANPC_AIController>(OwnerComp.GetAIOwner());
	auto const* const NPC = Cast<ANPC>(Cont->GetPawn());

	//get Player Character
	auto const* const Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	//write true and false to the blackboard key depending on whether or not the player is in range 
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), NPC->GetDistanceTo(Player) <= MeleeRange);

}
