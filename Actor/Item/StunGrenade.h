// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HorrorGame/Actor/Item.h"
#include "StunGrenade.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API AStunGrenade : public AItem
{
	GENERATED_BODY()
public:
	AStunGrenade();

	virtual void UseItem() override;
};
