#include "LockerActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ALockerActor::ALockerActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Door mesh: attach to pivot
    LockerDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockerDoorMesh"));
    LockerDoorMesh->SetupAttachment(DoorPivot);
    LockerDoorMesh->SetRelativeLocation(FVector(0.f, -50.f, 0.f));
    LockerDoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LockerDoorMesh->SetCollisionObjectType(ECC_WorldStatic);
    LockerDoorMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Entry & exit points
    EntryPoint = CreateDefaultSubobject<USceneComponent>(TEXT("EntryPoint"));
    EntryPoint->SetupAttachment(RootComponent);
    ExitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ExitPoint"));
    ExitPoint->SetupAttachment(RootComponent);

    // Door rotations
    DoorClosedRotation = FRotator::ZeroRotator;
    DoorOpenRotation = FRotator(0.f, 90.f, 0.f);

    // Audio Component
    DoorAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("DoorAudioComp"));
    DoorAudioComp->SetupAttachment(LockerDoorMesh);
    DoorAudioComp->bAutoActivate = false;

    bPlayerHidden = false;
    HiddenPlayer = nullptr;
}

void ALockerActor::BeginPlay()
{
    Super::BeginPlay();

    if (DoorOpenCurve && DoorTimeline)
    {
        // Progress callback
        FOnTimelineFloat Progress;
        Progress.BindUFunction(this, FName("HandleDoorProgress"));
        DoorTimeline->AddInterpFloat(DoorOpenCurve, Progress);

        // Finished callback
        FOnTimelineEvent Finished;
        Finished.BindUFunction(this, FName("OnDoorTimelineFinished"));
        DoorTimeline->SetTimelineFinishedFunc(Finished);

        DoorTimeline->SetLooping(false);
    }
}

void ALockerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (DoorTimeline) DoorTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, nullptr);
}

void ALockerActor::OnDoorTimelineFinished()
{
    // If reached open position
    float Pos = DoorTimeline->GetPlaybackPosition();
    float Len = DoorTimeline->GetTimelineLength();

    if (FMath::IsNearlyEqual(Pos, Len, 1e-3f))
    {
        // Play close sound
        PlayDoorSound(CloseDoorSound);

        // Toggle hidden state and teleport
        if (!bPlayerHidden && HiddenPlayer)
        {
            HiddenPlayer->SetActorLocation(EntryPoint->GetComponentLocation());
            HiddenPlayer->GetCharacterMovement()->DisableMovement();
            bPlayerHidden = true;
        }
        else if (bPlayerHidden && HiddenPlayer)
        {
            HiddenPlayer->SetActorLocation(ExitPoint->GetComponentLocation());
            HiddenPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            bPlayerHidden = false;
            HiddenPlayer = nullptr;
        }

        // Reverse to close door
        DoorTimeline->Reverse();
    }
}

void ALockerActor::PlayDoorSound(USoundBase* Sound)
{
    if (DoorAudioComp && Sound)
    {
        DoorAudioComp->SetSound(Sound);
        DoorAudioComp->Play();
    }
}

bool ALockerActor::CanOpenDoor_Implementation(AHorrorGameCharacter* Player)
{
    if (!Player || !DoorTimeline) return false;

    // Set the player for callback
    if (!bPlayerHidden)
    {
        HiddenPlayer = Player;
        return true;
    }

    if (DoorTimeline)
    {
        // Always play open animation and sound
        DoorTimeline->PlayFromStart();
        PlayDoorSound(OpenDoorSound);
        return true;
    }

    return false;
}

void ALockerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALockerActor, bPlayerHidden);
}

void ALockerActor::OnRep_PlayerHidden()
{
	LockerDoorMesh->SetVisibility(bPlayerHidden);
}
