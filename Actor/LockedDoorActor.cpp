// LockedDoorActor.cpp

#include "LockedDoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Item/Keys.h"
#include "HorrorGame/Widget/KeyNotificationWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ALockedDoorActor::ALockedDoorActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root pivot for hinge
    DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
    RootComponent = DoorPivot;

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

    // Lấy closed rotation
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
    if (!Player)
        return;

    if (bIsLocked)
    {
        // Chưa mở khóa: check chìa
        for (AActor* Item : Player->Inventory)
        {
            if (AKeys* Key = Cast<AKeys>(Item))
            {
                if (Key->KeyID == RequiredKeyID)
                {
                    UnlockDoor(Player);
                    return;
                }
            }
        }

        // Không có chìa báo UI
        if (Player->KeyNotificationWidget)
        {
            Player->KeyNotificationWidget->UpdateKeyNotification(RequiredKeyID.ToString());
        }
    }
    else
    {
        // Đã mở khóa, nhưng chưa mở cửa
        if (!bHasOpened)
        {
            PlayOpenDoorAnim();
            bHasOpened = true;
        }
        // Nếu muốn hỗ trợ đóng/chuyển trạng thái => thêm logic ở đây
    }
}

void ALockedDoorActor::UnlockDoor(AHorrorGameCharacter* Player)
{
    // Gỡ chìa ra khỏi inventory
    for (int32 i = Player->Inventory.Num() - 1; i >= 0; --i)
    {
        if (AKeys* Key = Cast<AKeys>(Player->Inventory[i]))
        {
            if (Key->KeyID == RequiredKeyID)
            {
                Key->Destroy();
                Player->Inventory.RemoveAt(i);
                break;
            }
        }
    }

    // Unlock và ẩn ổ khóa
    bIsLocked = false;
    LockMesh->SetVisibility(false);

    // Thông báo mở khóa thành công
    if (Player->KeyNotificationWidget)
    {
        Player->KeyNotificationWidget->UpdateKeyNotification(TEXT("Mở khóa thành công!"));
    }
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
