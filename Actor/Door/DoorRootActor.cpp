#include "HorrorGame/Actor/Door/DoorRootActor.h"
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

    // Create root component (không xoay)
    DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
    RootComponent = DoorPivot;

    // Create door frame (attach trực tiếp vào root, không bị xoay)
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(RootComponent);

    // Create door mesh pivot (đây mới là điểm xoay cho door mesh)
    DoorMeshPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorMeshPivot"));
    DoorMeshPivot->SetupAttachment(RootComponent);

    // Create door mesh (attach vào DoorMeshPivot)
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(DoorMeshPivot);

    DoorMeshPivot->SetRelativeLocation(FVector(0.0f, 50.0f,0.0f));

    DoorMesh->SetRelativeLocation(FVector(0.0f, -6.0f, 0.0f));

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

    // Store initial rotation as closed state (của DoorMeshPivot, không phải DoorPivot)
    ClosedRotation = DoorMeshPivot->GetRelativeRotation();
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

void ADoorRootActor::SetDoorPivotPosition(FVector PivotOffsets)
{
    if (DoorMeshPivot && DoorMesh)
    {
        DoorMeshPivot->SetRelativeLocation(PivotOffsets);
        // Offset ngược lại cho door mesh để giữ vị trí gốc
        DoorMesh->SetRelativeLocation(-PivotOffsets);
    }
}

void ADoorRootActor::AutoSetHingePivot(bool bLeftHinge)
{
    if (!DoorMesh || !DoorMesh->GetStaticMesh()) return;

    // Lấy bounds của door mesh
    FBoxSphereBounds MeshBounds = DoorMesh->GetStaticMesh()->GetBounds();
    FVector MeshExtent = MeshBounds.BoxExtent;

    // Tính toán vị trí bản lề
    FVector HingeOffset;
    if (bLeftHinge)
    {
        // Bản lề bên trái (nhìn từ phía trước cửa)
        HingeOffset = FVector(0.0f, -MeshExtent.Y, 0.0f);
    }
    else
    {
        // Bản lề bên phải
        HingeOffset = FVector(0.0f, MeshExtent.Y, 0.0f);
    }

    SetDoorPivotPosition(HingeOffset);

    UE_LOG(LogTemp, Log, TEXT("Auto-set hinge pivot: %s, Offset: %s"),
        bLeftHinge ? TEXT("Left") : TEXT("Right"),
        *HingeOffset.ToString());
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

    CurrentPlayer = Player;

    // CHỈ tính toán hướng cửa khi chưa được set HOẶC khi cửa đang đóng hoàn toàn
    if (!bDoorDirectionSet || bIsDoorClosed)
    {
        SetDoorSameSide();
        bDoorDirectionSet = true;
        UE_LOG(LogTemp, Log, TEXT("Setting door direction - Same side: %s"),
            bDoorOnSameSide ? TEXT("true") : TEXT("false"));
    }

    OnDoorInteraction(Player);
    bIsDoorClosed = !bIsDoorClosed;
    bIsAnimating = true;

    OnRep_DoorStateChanged();
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
    if (!DoorMeshPivot) return;

    FRotator NewRotation = CalculateDoorRotation(Value);
    DoorMeshPivot->SetRelativeRotation(NewRotation);
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
    DOREPLIFETIME(ADoorRootActor, bDoorDirectionSet);
}