// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SettingsController.generated.h"

UCLASS()
class HORRORGAME_API ASettingsController : public APlayerController
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditDefaultsOnly,Category = "Input | Yaw")
	float InputYawScale;

	UPROPERTY(EditDefaultsOnly,Category = "Input | Pitch")
	float InputPitchScale;

public:
	UFUNCTION()
	float getInputYawnScale() { return InputYawScale; }

	UFUNCTION()
	float getInputPitchScale() { return InputPitchScale; }

	UFUNCTION()
	float setInputYawScale(float YawScale) { return InputYawScale = YawScale; }

	UFUNCTION()
	float setInputPitchScale(float PitchScale) { return InputPitchScale = PitchScale; }
};
