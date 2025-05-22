// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HorrorGame/Actor/Item.h"
#include "FlashLight.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API AFlashLight : public AItem
{
	GENERATED_BODY()
public:
	AFlashLight();

	virtual void UseItem() override;
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UFlashLightComponent* FlashLightComp;

	bool bFlashAttached = false;
};
