#include "JumpScareActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// Sets default values
AJumpScareActor::AJumpScareActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetVisibility(false);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	StartLocation = CreateDefaultSubobject<USceneComponent>(TEXT("StartLocation"));
	StartLocation->SetupAttachment(RootComponent);

	EndLocation = CreateDefaultSubobject<USceneComponent>(TEXT("EndLocation"));
	EndLocation->SetupAttachment(RootComponent);

	JumpScareSpeed = 1000.0f;
	FinishDistance = 50.0f;

	bIsJumpScareActive = false;
	bSoundPlayed = false;
	MoveDirection = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void AJumpScareActor::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AJumpScareActor::OverlapBegin);
	}

	// Set mesh to start pos if available
	if (StartLocation && Mesh)
	{
		Mesh->SetWorldLocation(StartLocation->GetComponentLocation());
	}
}

// Called every frame
void AJumpScareActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsJumpScareActive)
		return;

	if (!Mesh || !EndLocation)
		return;

	FVector Current = Mesh->GetComponentLocation();
	FVector NewLocation = Current + MoveDirection * JumpScareSpeed * DeltaTime;
	Mesh->SetWorldLocation(NewLocation);

	// play sound once at the start of the jumpscare
	if (!bSoundPlayed && JumpScareSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JumpScareSound, NewLocation);
		bSoundPlayed = true;
	}

	// check finished using squared distance for performance
	const float DistSq = (EndLocation->GetComponentLocation() - NewLocation).SizeSquared();
	if (DistSq <= FMath::Square(FinishDistance))
	{
		CompleteJumpScare();
	}
}

void AJumpScareActor::TriggerJumpScare()
{
	if (!StartLocation || !EndLocation || !Mesh)
		return;

	// If already active, ignore (or change to restart behavior if you want)
	if (bIsJumpScareActive)
		return;

	// chuẩn bị
	Mesh->SetVisibility(true);
	Mesh->SetWorldLocation(StartLocation->GetComponentLocation());

	const FVector Start = StartLocation->GetComponentLocation();
	const FVector End = EndLocation->GetComponentLocation();
	MoveDirection = (End - Start).GetSafeNormal();

	bIsJumpScareActive = true;
	bSoundPlayed = false;
}

void AJumpScareActor::CompleteJumpScare()
{
	if (!bIsJumpScareActive)
		return;

	// stop and hide
	bIsJumpScareActive = false;
	Mesh->SetVisibility(false);
	bSoundPlayed = false;

	// reset to start location (optional)
	if (StartLocation && Mesh)
	{
		Mesh->SetWorldLocation(StartLocation->GetComponentLocation());
	}

	// broadcast event for Blueprints/other listeners
	if (OnJumpScareCompleted.IsBound())
	{
		OnJumpScareCompleted.Broadcast();
	}
}

void AJumpScareActor::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (PlayerCharacter)
	{
		TriggerJumpScare();
	}
}
