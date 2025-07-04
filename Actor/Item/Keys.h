// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HorrorGame/Actor/Item.h"
#include "Keys.generated.h"

UCLASS()
class HORRORGAME_API AKeys : public AItem
{
	GENERATED_BODY()
	
public:
	AKeys();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Key")
	FName KeyID;
};
