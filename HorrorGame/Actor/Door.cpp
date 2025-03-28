#include "HorrorGame/Actor/Door.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create and attach the door frame
	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	SetRootComponent(DoorFrame);

	// Create and attach the door
	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(DoorFrame);

	// Create and attach the collision box
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	BoxCollision->SetupAttachment(DoorFrame);

	REFInside = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("REFInside"));
	REFInside->SetupAttachment(DoorFrame);
	REFInside->bHiddenInGame = true;

	REFOutside = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("REFOutside"));
	REFOutside->SetupAttachment(DoorFrame);
	REFOutside->bHiddenInGame = true;

	bIsDoorClose = true; // Door is initially closed
	bIsPlayerInside = false; // Player starts outside
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the timeline if the curve is set
	if (CurveFloat)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("OpenDoor"));
		Timeline.AddInterpFloat(CurveFloat, TimelineProgress);
	}
}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update the timeline
	Timeline.TickTimeline(DeltaTime);
}

void ADoor::Interact()
{
	if (bIsDoorClose)
	{
		UE_LOG(LogTemp, Warning, TEXT("Door is opening"));
		Timeline.Play();
		bIsDoorClose = false;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Door is closing"));
		Timeline.Reverse();
		bIsDoorClose = true;
	}

	// Get the player
	Player = Cast<AHorrorGameCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (Player)
	{
		// Toggle between REFInside and REFOutside
		if (!bIsPlayerInside)
		{
			Player->SetActorLocation(REFInside->GetComponentLocation());
			bIsPlayerInside = true;
			UE_LOG(LogTemp, Warning, TEXT("Player moved to REFInside"));
			Player->GetCharacterMovement()->DisableMovement();
		}
		else
		{
			Player->SetActorLocation(REFOutside->GetComponentLocation());
			bIsPlayerInside = false;
			UE_LOG(LogTemp, Warning, TEXT("Player moved to REFOutside"));
			Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
}

void ADoor::OpenDoor(float Value)
{
	float Angle = bDoorOnSameSide ? -DoorRotateAngle : DoorRotateAngle;
	FRotator Rot = FRotator(0.f, Angle * Value, 0.f);
	Door->SetRelativeRotation(Rot);
}

void ADoor::SetDoorSameSide()
{
	if (Player)
	{
		// Calculate vectors for door and player
		FVector PlayerForward = Player->GetActorForwardVector();
		FVector DoorForward = GetActorForwardVector();

		// Determine if the player is on the same side
		bDoorOnSameSide = FVector::DotProduct(PlayerForward, DoorForward) >= 0;
	}
}
