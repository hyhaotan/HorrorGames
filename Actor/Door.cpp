#include "HorrorGame/Actor/Door.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"
#include "HorrorGame/HorrorGameCharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/WidgetAnimation.h"

ADoor::ADoor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create components
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    SetRootComponent(DoorFrame);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Mesh->SetupAttachment(DoorFrame);

    // Default values
    DoorRotateAngle = 90.f;
    bIsDoorClosed = true;
    bDoorOnSameSide = false;
    Players = nullptr;
}

void ADoor::BeginPlay()
{
    Super::BeginPlay();

    if (CurveFloat)
    {
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("OpenDoor"));
        DoorTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
    }
}

void ADoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    DoorTimeline.TickTimeline(DeltaTime);
}

void ADoor::Interact(AHorrorGameCharacter* Player)
{
    if (!Player) return;

    SetDoorSameSide();
    if (bIsDoorClosed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Door is opening"));
        DoorTimeline.Play();
        bIsDoorClosed = false;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Door is closing"));
        DoorTimeline.Reverse();
        bIsDoorClosed = true;
    }
}

void ADoor::OpenDoor(float Value)
{
    const float Angle = bDoorOnSameSide ? -DoorRotateAngle : DoorRotateAngle;
    Mesh->SetRelativeRotation(FRotator(0.f, Angle * Value, 0.f));
}

void ADoor::SetDoorSameSide()
{
    if (!Players) return;

    FVector DoorToPlayer = Players->GetActorLocation() - GetActorLocation();
    FVector DoorForward = GetActorForwardVector();

    bDoorOnSameSide = FVector::DotProduct(DoorToPlayer, DoorForward) >= 0;
}
