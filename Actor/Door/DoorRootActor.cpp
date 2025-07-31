// Fill out your copyright notice in the Description page of Project Settings.

#include "DoorRootActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADoorRootActor::ADoorRootActor()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Root pivot for hinge
    DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
    RootComponent = DoorPivot;

    // Door mesh
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(DoorPivot);

    // Timeline component
    DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

    // Load default curve
    static ConstructorHelpers::FObjectFinder<UCurveFloat> CurveObj(TEXT("/Game/CurveFloat/DoorCurve.DoorCurve"));
    if (CurveObj.Succeeded())
    {
        DoorOpenCurve = CurveObj.Object;
        UE_LOG(LogTemp, Log, TEXT("DoorOpenCurve loaded OK"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load DoorOpenCurve!"));
    }

    // Initialize state
    bHasOpened = false;
    ClosedRotation = FRotator::ZeroRotator;
    OpenRotation = FRotator(0.f, 90.f, 0.f);
}

// Called when the game starts or when spawned
void ADoorRootActor::BeginPlay()
{
    Super::BeginPlay();
    
    ClosedRotation = DoorPivot->GetRelativeRotation();
    OpenRotation = ClosedRotation + FRotator(0.f, 90.f, 0.f);

    if (DoorOpenCurve && DoorTimeline)
    {
        FOnTimelineFloat ProgressDelegate;
        ProgressDelegate.BindUFunction(this, FName("HandleDoorProgress"));
        DoorTimeline->AddInterpFloat(DoorOpenCurve, ProgressDelegate);
        DoorTimeline->SetLooping(false);
        DoorTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
    }
}

// Called every frame
void ADoorRootActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (DoorTimeline)
    {
        DoorTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, nullptr);
    }
}

void ADoorRootActor::HandleDoorProgress(float Value)
{
    FRotator NewRotation = FMath::Lerp(ClosedRotation, OpenRotation, Value);
    DoorPivot->SetRelativeRotation(NewRotation);
}

void ADoorRootActor::Interact(AHorrorGameCharacter* Player)
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

bool ADoorRootActor::ServerInteract_Validate(AHorrorGameCharacter* Player)
{
    return true;
}

void ADoorRootActor::ServerInteract_Implementation(AHorrorGameCharacter* Player)
{
    if (!Player) return;

    if (!CanOpenDoor(Player))
    {
        return;
    }
    
    if (!bHasOpened)
    {
        bHasOpened = true;
        PlayOpenDoorAnim();
    }
}

void ADoorRootActor::PlayOpenDoorAnim()
{
    if (DoorTimeline && DoorOpenCurve)
    {
        UE_LOG(LogTemp, Log, TEXT("Playing door timeline..."));
        DoorTimeline->PlayFromStart();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play timeline: %s %s"),
            DoorTimeline ? TEXT("has Timeline") : TEXT("no Timeline"),
            DoorOpenCurve ? TEXT("") : TEXT("no Curve"));
    }
}

bool ADoorRootActor::CanOpenDoor_Implementation(AHorrorGameCharacter* Player)
{
    return true; // Base implementation always allows opening
}

void ADoorRootActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADoorRootActor, bHasOpened);
}
