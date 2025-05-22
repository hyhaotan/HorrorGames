// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HorrorGame/Actor/Item.h"
#include "MolotovCocktail.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API AMolotovCocktail : public AItem
{
	GENERATED_BODY()

public:
	AMolotovCocktail();

	virtual void UseItem() override;
	
};
