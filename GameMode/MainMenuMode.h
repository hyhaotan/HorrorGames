// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuMode.generated.h"

UCLASS()
class HORRORGAME_API AMainMenuMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AMainMenuMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideAllGameWidgets();

};
