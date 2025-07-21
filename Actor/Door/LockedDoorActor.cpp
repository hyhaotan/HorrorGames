#include "LockedDoorActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Item/Keys.h"
#include "HorrorGame/Widget/KeyNotificationWidget.h"
#include "HorrorGame/Widget/Inventory/Inventory.h"

#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

ALockedDoorActor::ALockedDoorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Root pivot for hinge
    DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
    DoorPivot->SetupAttachment(RootComponent);

    // Door mesh
    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(DoorPivot);

    // Lock mesh
    LockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockMesh"));
    LockMesh->SetupAttachment(DoorMesh);

    // Timeline component
    DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

    // Tải curve từ content
    static ConstructorHelpers::FObjectFinder<UCurveFloat> CurveObj(TEXT("/Game/CurveFloat/DoorCurve.DoorCurve"));
    if (CurveObj.Succeeded()) {
        DoorOpenCurve = CurveObj.Object;
        UE_LOG(LogTemp, Log, TEXT("DoorOpenCurve loaded OK"));
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DoorOpenCurve!"));
    }

    // Khởi tạo trạng thái
    RequiredKeyID = NAME_None;
    bIsLocked = true;
    bHasOpened = false;
    ClosedRotation = FRotator::ZeroRotator;
    OpenRotation = FRotator(0.f, 90.f, 0.f);
}

void ALockedDoorActor::BeginPlay()
{
    Super::BeginPlay();

    ClosedRotation = DoorPivot->GetRelativeRotation();
    OpenRotation = ClosedRotation + FRotator(0.f, 90.f, 0.f);

    // Hiển thị ổ khóa nếu còn khóa
    LockMesh->SetVisibility(bIsLocked);

    if (DoorOpenCurve && DoorTimeline)
    {
        FOnTimelineFloat ProgressDelegate;
        ProgressDelegate.BindUFunction(this, FName("HandleDoorProgress"));
        DoorTimeline->AddInterpFloat(DoorOpenCurve, ProgressDelegate);
        DoorTimeline->SetLooping(false);
        DoorTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
    }
}

void ALockedDoorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (DoorTimeline)
    {
        DoorTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, nullptr);
    }
}

void ALockedDoorActor::HandleDoorProgress(float Value)
{
    FRotator NewRotation = FMath::Lerp(ClosedRotation, OpenRotation, Value);
    DoorPivot->SetRelativeRotation(NewRotation);
}

void ALockedDoorActor::Interact(AHorrorGameCharacter* Player)
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

bool ALockedDoorActor::ServerInteract_Validate(AHorrorGameCharacter* Player)
{
    return true;
}

void ALockedDoorActor::ServerInteract_Implementation(AHorrorGameCharacter* Player)
{
    if (!Player) return;

    if (bIsLocked)
    {
        if (Player->bIsHoldingItem && Player->EquippedItem)
        {
            if (AKeys* Key = Cast<AKeys>(Player->EquippedItem))
            {
                UnlockDoor(Player);
                LockMesh->SetVisibility(false, true);
                bIsLocked = false;
                OnRep_IsLocked();
            }
        }

        if (Player->KeyNotificationWidget)
        {
            Player->KeyNotificationWidget->UpdateKeyNotification(RequiredKeyID.ToString());
        }
        return;
    }
    
    if (!bHasOpened)
    {
        bHasOpened = true;
        Multicast_PlayOpenDoor();
    }
}

void ALockedDoorActor::UnlockDoor(AHorrorGameCharacter* Player)
{
    if (Player->EquippedItem)
    {
        if (AKeys* Key = Cast<AKeys>(Player->EquippedItem))
        {
            if (Key->KeyID == RequiredKeyID)
            {
                Player->StoreCurrentHeldObject();

                Key->Destroy();

                Player->Inventory.Remove(Key);

                if (Player->InventoryWidget)
                    Player->InventoryWidget->UpdateInventory(Player->Inventory);

                Player->OnInventoryUpdated.Broadcast(Player->Inventory);
            }
        }
    }

    if (Player->KeyNotificationWidget)
    {
        Player->KeyNotificationWidget->UpdateKeyNotification(TEXT("Mở khóa thành công!"));
    }
}

void ALockedDoorActor::Multicast_PlayOpenDoor_Implementation()
{
    PlayOpenDoorAnim();
}

void ALockedDoorActor::PlayOpenDoorAnim()
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

void ALockedDoorActor::OnRep_IsLocked()
{
    LockMesh->SetVisibility(bIsLocked);
}

void ALockedDoorActor::OnRep_HasOpened()
{
    if (bHasOpened)
    {
        PlayOpenDoorAnim();
    }
}

void ALockedDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALockedDoorActor, bIsLocked);
    DOREPLIFETIME(ALockedDoorActor, bHasOpened);
}