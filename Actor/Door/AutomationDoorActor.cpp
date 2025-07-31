#include "AutomationDoorActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AAutomationDoorActor::AAutomationDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMesh"));
	RootComponent = DoorFrameMesh;

	LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoorMesh"));
	LeftDoorMesh->SetupAttachment(DoorFrameMesh);

	RightDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoorMesh"));
	RightDoorMesh->SetupAttachment(DoorFrameMesh);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(DoorFrameMesh);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

	OpenDistance = 100.0f;
	bIsOpen = false;
	OverlapCount = 0;

	ConstructorHelpers::FObjectFinder<UCurveFloat> DoorOpenCurveAsset(TEXT("/Game/CurveFloat/DoorCurve.DoorCurve"));
	if (DoorOpenCurveAsset.Succeeded())
	{
		DoorOpenCurve = DoorOpenCurveAsset.Object;
	}
	else
	{
		DoorOpenCurve = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Door Open Curve not found!"));
	}
}

void AAutomationDoorActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AAutomationDoorActor::OnOverlapBegin);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AAutomationDoorActor::OnOverlapEnd);
	}

	if (DoorOpenCurve && DoorTimeline)
	{
		FOnTimelineFloat Progress;
		Progress.BindUFunction(this, FName("HandleDoorTimeline"));
		DoorTimeline->AddInterpFloat(DoorOpenCurve, Progress);
		DoorTimeline->SetLooping(false);
	}
}

void AAutomationDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DoorTimeline)
	{
		DoorTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	}
}

void AAutomationDoorActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && Cast<AHorrorGameCharacter>(OtherActor))
	{
		OverlapCount++;
		MulticastOpenDoor();
		UE_LOG(LogTemp, Warning, TEXT("Overlapping with: %s"), *OtherActor->GetName());
	}
}

void AAutomationDoorActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	if (HasAuthority() && Cast<AHorrorGameCharacter>(OtherActor))
	{
		OverlapCount--;
		if (OverlapCount <= 0)
		{
			OverlapCount = 0;
			MulticastCloseDoor();
			UE_LOG(LogTemp, Warning, TEXT("UnOverlapping with : %s"), *OtherActor->GetName());
		}
	}
}

void AAutomationDoorActor::MulticastOpenDoor_Implementation()
{
	if (!bIsOpen)
	{
		bIsOpen = true;
		DoorTimeline->PlayFromStart();
	}
}

void AAutomationDoorActor::MulticastCloseDoor_Implementation()
{
	if (bIsOpen)
	{
		bIsOpen = false;
		DoorTimeline->Reverse();
	}
}

void AAutomationDoorActor::HandleDoorTimeline(float Value)
{
	FVector LeftTarget = FMath::Lerp(FVector::ZeroVector, FVector(-OpenDistance, 0, 0), Value);
	FVector RightTarget = FMath::Lerp(FVector::ZeroVector, FVector(OpenDistance, 0, 0), Value);
	LeftDoorMesh->SetRelativeLocation(LeftTarget);
	RightDoorMesh->SetRelativeLocation(RightTarget);
}

void AAutomationDoorActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAutomationDoorActor, bIsOpen);
}