#include "DoorRootActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TimelineComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

ADoorRootActor::ADoorRootActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Create door pivot (hinge point)
    DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
    RootComponent = DoorPivot;

    // Create door mesh
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(DoorPivot);

    // Create optional door frame
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(RootComponent);

    // Create timeline component
    DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

    // Try to load default curve
    static ConstructorHelpers::FObjectFinder<UCurveFloat> CurveObj(TEXT("/Game/CurveFloat/DoorCurve.DoorCurve"));
    if (CurveObj.Succeeded())
    {
        DoorOpenCurve = CurveObj.Object;
    }

    // Initialize default values
    DoorRotateAngle = 90.0f;
    bIsDoorClosed = true;
    bIsAnimating = false;
    bDoorOnSameSide = false;
    CurrentPlayer = nullptr;

    ClosedRotation = FRotator::ZeroRotator;
    OpenRotation = FRotator(0.0f, 90.0f, 0.0f);

    // Hide base mesh if it exists
    if (Mesh)
    {
        Mesh->SetVisibility(false);
    }
}

void ADoorRootActor::BeginPlay()
{
    Super::BeginPlay();

    // Store initial rotation as closed state
    ClosedRotation = DoorPivot->GetRelativeRotation();
    OpenRotation = ClosedRotation + FRotator(0.0f, DoorRotateAngle, 0.0f);

    // Setup timeline
    SetupTimeline();
}

void ADoorRootActor::SetupTimeline()
{
    if (DoorOpenCurve)
    {
        // Setup UTimelineComponent
        if (DoorTimeline)
        {
            FOnTimelineFloat ProgressDelegate;
            ProgressDelegate.BindUFunction(this, FName("HandleDoorProgress"));
            DoorTimeline->AddInterpFloat(DoorOpenCurve, ProgressDelegate);
            DoorTimeline->SetLooping(false);
            DoorTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
        }

        // Setup custom FTimeline for more advanced control
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("HandleDoorProgress"));

        FOnTimelineEvent TimelineFinished;
        TimelineFinished.BindUFunction(this, FName("OnDoorAnimationFinished"));

        CustomDoorTimeline.AddInterpFloat(DoorOpenCurve, TimelineProgress);
        CustomDoorTimeline.SetTimelineFinishedFunc(TimelineFinished);
        CustomDoorTimeline.SetLooping(false);
    }
}

void ADoorRootActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Tick both timelines
    if (DoorTimeline)
    {
        DoorTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, nullptr);
    }

    CustomDoorTimeline.TickTimeline(DeltaTime);
}

void ADoorRootActor::Interact(AHorrorGameCharacter* Player)
{
    if (HasAuthority())
    {
        ServerDoorInteract_Implementation(Player);
    }
    else
    {
        ServerDoorInteract(Player);
    }
}

bool ADoorRootActor::ServerDoorInteract_Validate(AHorrorGameCharacter* Player)
{
    return Player != nullptr;
}

void ADoorRootActor::ServerDoorInteract_Implementation(AHorrorGameCharacter* Player)
{
    if (!Player || bIsAnimating)
    {
        return;
    }

    if (!CanOpenDoor(Player))
    {
        return;
    }

    // Set current player and calculate side
    CurrentPlayer = Player;
    SetDoorSameSide();

    // Call the overridable interaction event
    OnDoorInteraction(Player);

    // Default behavior: toggle door state
    bIsDoorClosed = !bIsDoorClosed;
    bIsAnimating = true;

    UE_LOG(LogTemp, Log, TEXT("Door %s on server"),
        bIsDoorClosed ? TEXT("closing") : TEXT("opening"));

    // Trigger replication
    OnRep_DoorStateChanged();

    // Play animation on server
    PlayDoorAnimation();
}

void ADoorRootActor::OnRep_DoorStateChanged()
{
    UE_LOG(LogTemp, Log, TEXT("Door state replicated: %s"),
        bIsDoorClosed ? TEXT("closing") : TEXT("opening"));

    // Play animation on clients
    if (!HasAuthority())
    {
        PlayDoorAnimation();
    }
}

bool ADoorRootActor::CanOpenDoor_Implementation(AHorrorGameCharacter* Player)
{
    // Base implementation - always allow opening
    // Override this in child classes for specific requirements
    return true;
}

void ADoorRootActor::OnDoorInteraction_Implementation(AHorrorGameCharacter* Player)
{
    // Base implementation - empty
    // Override this in child classes for custom interaction behavior
    UE_LOG(LogTemp, Log, TEXT("Door interaction occurred with player: %s"),
        Player ? *Player->GetName() : TEXT("Unknown"));
}

void ADoorRootActor::PlayDoorAnimation_Implementation()
{
    if (!DoorOpenCurve)
    {
        UE_LOG(LogTemp, Warning, TEXT("No door curve found for %s"), *GetName());
        bIsAnimating = false;
        return;
    }

    if (bIsDoorClosed)
    {
        // Closing: reverse the timeline
        CustomDoorTimeline.Reverse();
        UE_LOG(LogTemp, Log, TEXT("Playing door close animation"));
    }
    else
    {
        // Opening: play from start
        CustomDoorTimeline.PlayFromStart();
        UE_LOG(LogTemp, Log, TEXT("Playing door open animation"));
    }
}

void ADoorRootActor::HandleDoorProgress(float Value)
{
    if (!DoorPivot) return;

    FRotator NewRotation = CalculateDoorRotation(Value);
    DoorPivot->SetRelativeRotation(NewRotation);
}

FRotator ADoorRootActor::CalculateDoorRotation(float AnimationValue)
{
    // Calculate rotation based on side and animation value
    const float Angle = bDoorOnSameSide ? -DoorRotateAngle : DoorRotateAngle;
    FRotator TargetRotation = ClosedRotation + FRotator(0.0f, Angle, 0.0f);

    // Lerp between closed and target rotation
    return FMath::Lerp(ClosedRotation, TargetRotation, AnimationValue);
}

void ADoorRootActor::OnDoorAnimationFinished()
{
    bIsAnimating = false;
    CurrentPlayer = nullptr;

    UE_LOG(LogTemp, Log, TEXT("Door animation finished. Door is now %s"),
        bIsDoorClosed ? TEXT("closed") : TEXT("open"));
}

void ADoorRootActor::SetDoorSameSide()
{
    if (!CurrentPlayer) return;

    FVector DoorToPlayer = CurrentPlayer->GetActorLocation() - GetActorLocation();
    FVector DoorForward = GetActorForwardVector();
    bDoorOnSameSide = FVector::DotProduct(DoorToPlayer, DoorForward) >= 0;

    UE_LOG(LogTemp, Log, TEXT("Door side calculated: %s"),
        bDoorOnSameSide ? TEXT("Same side") : TEXT("Opposite side"));
}

void ADoorRootActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADoorRootActor, bIsDoorClosed);
    DOREPLIFETIME(ADoorRootActor, bIsAnimating);
    DOREPLIFETIME(ADoorRootActor, bDoorOnSameSide);
}