// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorGameGameMode.h"
#include "HorrorGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

AHorrorGameGameMode::AHorrorGameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
