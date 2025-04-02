// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/AI/AttackAnimNotifyState.h"
#include "HorrorGame/AI/AICharacterBase.h"

void UAttackAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (AAICharacterBase* const Character = Cast<AAICharacterBase>(MeshComp->GetOwner()))
		{
			Character->AttackStart();
		}
	}
}

void UAttackAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	{
		if (MeshComp && MeshComp->GetOwner())
		{
			if (AAICharacterBase* const Character = Cast<AAICharacterBase>(MeshComp->GetOwner()))
			{
				Character->AttackEnd();
			}
		}
	}
}

