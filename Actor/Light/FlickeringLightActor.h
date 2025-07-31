// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Actor/Light/LightActor.h"
#include "FlickeringLightActor.generated.h"

UCLASS()
class HORRORGAME_API AFlickeringLightActor : public ALightActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlickeringLightActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

private:
	void ToggleFlicker();

	float GetRandomDelay() const;

	FTimerHandle FlickerTimerHandle;

};
