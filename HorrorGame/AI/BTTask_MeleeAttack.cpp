// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/AI/BTTask_MeleeAttack.h"
#include "Combat_Interface.h"
#include "AIController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Engine/LatentActionManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

UBTTask_MeleeAttack::UBTTask_MeleeAttack()
{
	NodeName = TEXT("Melee Attack");
}

EBTNodeResult::Type UBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//if we are out of range ,do not attack the player
	auto const OutOfRange = !OwnerComp.GetBlackboardComponent()->GetValueAsBool(GetSelectedBlackboardKey());
	if (OutOfRange)
	{
		//finish the task
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}

	//we are in range so get the AI'Controller  and the NPC isself
	auto const* const Cont = OwnerComp.GetAIOwner();
	auto* const NPC = Cast <ANPC>(Cont->GetPawn());

	//if the NPC supports the ICombatInterface,cast and call the Execute_MeleeAttack function
	if (auto* const icombat = Cast <ICombat_Interface>(NPC))
	{
		//necessary check to see if the montage has finished so we don't try and play it again 
		if (MontageHasFinished(NPC))
		{
			icombat->Execute_MeleeAttack(NPC);
		}
	}

	//finish the task
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}

bool UBTTask_MeleeAttack::MontageHasFinished(ANPC* const NPC)
{
	return NPC->GetMesh()->GetAnimInstance()->Montage_GetIsStopped(NPC->GetMontage());
}
