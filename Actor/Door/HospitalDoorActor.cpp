#include "HospitalDoorActor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Item/BoltCutter.h"
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "HorrorGame/Widget/KeyNotificationWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"

AHospitalDoorActor::AHospitalDoorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    // Setup meshes
    LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoorMesh"));
    LeftDoorMesh->SetupAttachment(RootComponent);
    RightDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoorMesh"));
    RightDoorMesh->SetupAttachment(RootComponent);
    ChainDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChainDoorMesh"));
    ChainDoorMesh->SetupAttachment(RootComponent);

    // Block all by default
    ChainDoorMesh->SetCollisionProfileName(TEXT("BlockAll"));

    // Timeline
    OpenCloseTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("OpenCloseTimeline"));

    // Initial state
    bIsOpen = false;
    bIsLocked = true;
    bHasOpened = false;

    InitialLeftDoorRotation = FRotator::ZeroRotator;
    InitialRightDoorRotation = FRotator::ZeroRotator;
}

void AHospitalDoorActor::BeginPlay()
{
    Super::BeginPlay();

    // Cache initial rotations
    InitialLeftDoorRotation = LeftDoorMesh->GetRelativeRotation();
    InitialRightDoorRotation = RightDoorMesh->GetRelativeRotation();

    // Setup timeline
    if (OpenCloseTimeline && OpenCloseCurve)
    {
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("HandleOpenCloseDoor"));
        OpenCloseTimeline->AddInterpFloat(OpenCloseCurve, ProgressFunction);
        OpenCloseTimeline->SetLooping(false);
    }

    // Visual initial lock state
    ChainDoorMesh->SetVisibility(bIsLocked);
    ChainDoorMesh->SetCollisionEnabled(bIsLocked ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
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
        ServerInteract_Implementation(Player);
    else
        ServerInteract(Player);
}

void AHospitalDoorActor::ServerInteract_Implementation(AHorrorGameCharacter* Player)
{
    if (!Player) return;

    // Unlock stage
    if (bIsLocked)
    {
        if (Player->bIsHoldingItem && Player->EquippedItem)
        {
            if (ABoltCutter* Cutter = Cast<ABoltCutter>(Player->EquippedItem))
            {
                UnlockDoor(Player);
                bIsLocked = false;
                ChainDoorMesh->SetVisibility(false, true);
                ChainDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                return;
            }
        }
        // Show notification
        if (Player->KeyNotificationWidget)
            Player->KeyNotificationWidget->UpdateKeyNotification(TEXT("Bolt Cutter"));
        return;
    }

    // Open stage: only once
    if (!bHasOpened)
    {
        bIsOpen = true;
        bHasOpened = true;
        PlayOpenDoorAnim();
        UE_LOG(LogTemp, Warning, TEXT("The Door is opening!"));
    }
}

void AHospitalDoorActor::UnlockDoor(AHorrorGameCharacter* Player)
{
    if (ABoltCutter* Cutter = Cast<ABoltCutter>(Player->EquippedItem))
    {
        Player->StoreCurrentHeldObject();
        Cutter->Destroy();
        Player->Inventory.Remove(Cutter);

        if (UnlockDoorSequence)
        {
            ALevelSequenceActor* OutActor;
            ULevelSequencePlayer* PlayerSeq = ULevelSequencePlayer::CreateLevelSequencePlayer(
                GetWorld(), UnlockDoorSequence, FMovieSceneSequencePlaybackSettings(), OutActor);
            if (PlayerSeq)
                PlayerSeq->Play();
        }

        if (Player->InventoryWidget)
            Player->InventoryWidget->UpdateInventory(Player->Inventory);

        Player->OnInventoryUpdated.Broadcast(Player->Inventory);
    }
    if (Player->KeyNotificationWidget)
        Player->KeyNotificationWidget->UpdateKeyNotification(TEXT("Unlocked!"));
}

void AHospitalDoorActor::HandleOpenCloseDoor(float Value)
{
    float OpenAngle = 90.f * Value;
    LeftDoorMesh->SetRelativeRotation(InitialLeftDoorRotation + FRotator(0, -OpenAngle, 0));
    RightDoorMesh->SetRelativeRotation(InitialRightDoorRotation + FRotator(0, OpenAngle, 0));
}

void AHospitalDoorActor::OnRep_IsLocked()
{
    ChainDoorMesh->SetVisibility(bIsLocked, true);
    ChainDoorMesh->SetCollisionEnabled(bIsLocked ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void AHospitalDoorActor::OnRep_HasOpened()
{
    if (bHasOpened)
        PlayOpenDoorAnim();
}

void AHospitalDoorActor::PlayOpenDoorAnim()
{
    if (OpenCloseTimeline)
        OpenCloseTimeline->PlayFromStart();
}

void AHospitalDoorActor::OnRep_IsOpen()
{
    // Optional: mirror open state via timeline
    if (bIsOpen)
        PlayOpenDoorAnim();
}

void AHospitalDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AHospitalDoorActor, bIsOpen);
    DOREPLIFETIME(AHospitalDoorActor, bIsLocked);
    DOREPLIFETIME(AHospitalDoorActor, bHasOpened);
}
