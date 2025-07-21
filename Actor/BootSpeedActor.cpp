// Fill out your copyright notice in the Description page of Project Settings.


#include "BootSpeedActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ABootSpeedActor::ABootSpeedActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(MeshComponent);
	SphereCollision->SetupAttachment(MeshComponent);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->InitSphereRadius(100.0f); 

	SphereCollision->OnComponentBeginOverlap.AddDynamic(
		this,
		&ABootSpeedActor::OnOverlapBegin
	);
}

// Called when the game starts or when spawned
void ABootSpeedActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABootSpeedActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABootSpeedActor::ResetSpeed()
{
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed -= BootSpeed;
		Character = nullptr;
	}
	MeshComponent->SetVisibility(true);
}

void ABootSpeedActor::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	AHorrorGameCharacter* OverlappingChar = Cast<AHorrorGameCharacter>(OtherActor);
	if (OverlappingChar && OtherComponent)
	{
		OverlappingChar->GetCharacterMovement()->MaxWalkSpeed += BootSpeed;

		Character = OverlappingChar;

		MeshComponent->SetVisibility(false);

		// Schedule the reset
		GetWorldTimerManager().SetTimer(
			BootSpeedTimerHandle,
			this,
			&ABootSpeedActor::ResetSpeed,
			BootSpeedDuration,
			false
		);

		//SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
