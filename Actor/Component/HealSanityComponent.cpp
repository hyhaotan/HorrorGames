// Fill out your copyright notice in the Description page of Project Settings.


#include "HealSanityComponent.h"
#include "SanityComponent.h"

#include "GameFramework/Character.h"

// Sets default values for this component's properties
UHealSanityComponent::UHealSanityComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealSanityComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		SanityComponent = OwnerCharacter->FindComponentByClass<USanityComponent>();
	}
}


// Called every frame
void UHealSanityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

