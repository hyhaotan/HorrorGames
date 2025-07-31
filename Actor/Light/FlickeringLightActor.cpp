// FlickeringLightActor.cpp

#include "FlickeringLightActor.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AFlickeringLightActor::AFlickeringLightActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFlickeringLightActor::BeginPlay()
{
	Super::BeginPlay();

	// Bắt đầu timer flicker
	GetWorld()->GetTimerManager().SetTimer(
		FlickerTimerHandle,
		this,
		&AFlickeringLightActor::ToggleFlicker,
		GetRandomDelay(),
		false
	);
}

float AFlickeringLightActor::GetRandomDelay() const
{
	return FMath::RandRange(0.05f, 0.5f);
}

void AFlickeringLightActor::ToggleFlicker()
{
	const bool bNewVis = !SpotLight->IsVisible();
	SpotLight->SetVisibility(bNewVis);

	 float NewIntensity = bNewVis
	     ? FMath::RandRange(1000.f, 3000.f)
	     : 0.f;
	 SpotLight->SetIntensity(NewIntensity);

	GetWorld()->GetTimerManager().SetTimer(
		FlickerTimerHandle,
		this,
		&AFlickeringLightActor::ToggleFlicker,
		GetRandomDelay(),
		false
	);
}
