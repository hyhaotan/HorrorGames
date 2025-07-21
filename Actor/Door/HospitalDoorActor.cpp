#include "HorrorGame/Actor/Door/HospitalDoorActor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Item/BoltCutter.h"
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "HorrorGame/Widget/TextScreenWidget.h"
#include "Components/TimelineComponent.h"
#include "HorrorGame/Widget/KeyNotificationWidget.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"

AHospitalDoorActor::AHospitalDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoorMesh"));
	LeftDoorMesh->SetupAttachment(Mesh);

	RightDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoorMesh"));
	RightDoorMesh->SetupAttachment(Mesh);

	ChainDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChainDoorMesh"));
	ChainDoorMesh->SetupAttachment(Mesh);

	OpenCloseTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("OpenCloseTimeline"));

	InitialLeftDoorRotation = FRotator(0.0f, -90.0f, 0.0f);
	InitialRightDoorRotation = FRotator(0.0f, 90.0f, 0.0f);
	bIsOpen = false;
	bIsLocked = true;
}

void AHospitalDoorActor::BeginPlay()
{
	Super::BeginPlay();

	if (OpenCloseTimeline && OpenCloseCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleOpenCloseDoor"));
		OpenCloseTimeline->AddInterpFloat(OpenCloseCurve, ProgressFunction);
		OpenCloseTimeline->SetLooping(false);
	}

	InitialLeftDoorRotation = LeftDoorMesh->GetRelativeRotation() + FRotator(0.0f, -90.0f, 0.0f);
	InitialRightDoorRotation = RightDoorMesh->GetRelativeRotation() + FRotator(0.0f, 90.0f, 0.0f);

	ChainDoorMesh->SetVisibility(bIsLocked);
}

void AHospitalDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OpenCloseTimeline)
	{
		OpenCloseTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, nullptr);
	}
}

void AHospitalDoorActor::Interact(AHorrorGameCharacter* Player)
{
	if (HasAuthority())
	{
		ServerInteract_Implementation(Player);
	}
	else
	{
		ServerInteract(Player);
	}
}

bool AHospitalDoorActor::ServerInteract_Validate(AHorrorGameCharacter* Player)
{
	return true;
}

void AHospitalDoorActor::ServerInteract_Implementation(AHorrorGameCharacter* Player)
{
	if (bIsLocked)
	{
		if (Player->bIsHoldingItem && Player->EquippedItem)
		{
			if (ABoltCutter* Cutter = Cast<ABoltCutter>(Player->EquippedItem))
			{
				UnlockDoor(Player);
				ChainDoorMesh->SetVisibility(false, true);
				bIsLocked = false;
				return;
			}
		}

		if (Player->KeyNotificationWidget)
		{
			Player->KeyNotificationWidget->UpdateKeyNotification("Bolt Cutter");
		}
		return;
	}

	if (!bIsOpen && !bIsLocked)
	{
		bIsOpen = true;
		Multicast_PlayOpenDoor();
	}
}

void AHospitalDoorActor::UnlockDoor(AHorrorGameCharacter* Player)
{
	if (Player->EquippedItem)
	{
		if (ABoltCutter* Cutter = Cast<ABoltCutter>(Player->EquippedItem))
		{
			Player->StoreCurrentHeldObject();
			Cutter->Destroy();
			Player->Inventory.Remove(Cutter);

			if (UnlockDoorSequence)
			{
				ALevelSequenceActor* OutActor;
				ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
					GetWorld(), UnlockDoorSequence, FMovieSceneSequencePlaybackSettings(), OutActor);

				if (SequencePlayer)
				{
					SequencePlayer->Play();
				}
			}

			if (Player->InventoryWidget)
			{
				Player->InventoryWidget->UpdateInventory(Player->Inventory);
			}

			Player->OnInventoryUpdated.Broadcast(Player->Inventory);
		}
	}
}

void AHospitalDoorActor::Multicast_PlayOpenDoor_Implementation()
{
	if (OpenCloseTimeline)
	{
		OpenCloseTimeline->PlayFromStart();
	}
}

void AHospitalDoorActor::HandleOpenCloseDoor(float Value)
{
	float OpenAngle = 90.0f * Value;
	LeftDoorMesh->SetRelativeRotation(InitialLeftDoorRotation + FRotator(0, -OpenAngle, 0));
	RightDoorMesh->SetRelativeRotation(InitialRightDoorRotation + FRotator(0, OpenAngle, 0));
}

void AHospitalDoorActor::OnRep_IsLocked()
{
	ChainDoorMesh->SetVisibility(bIsLocked);
}

void AHospitalDoorActor::OnRep_IsOpen()
{
	if (bIsOpen && OpenCloseTimeline)
	{
		OpenCloseTimeline->PlayFromStart();
	}
}

void AHospitalDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHospitalDoorActor, bIsOpen);
	DOREPLIFETIME(AHospitalDoorActor, bIsLocked);
}
