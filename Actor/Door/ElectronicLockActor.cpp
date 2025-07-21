#include "ElectronicLockActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Widget/ElectronicLockWidget.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"
#include "HorrorGame/GameMode/MainMenuMode.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/TimelineComponent.h"

AElectronicLockActor::AElectronicLockActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root and mesh setup
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    RootComponent = DoorFrame;

    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Door->SetupAttachment(DoorFrame);

    Mesh->SetupAttachment(DoorFrame);

    ItemWidget->SetupAttachment(Mesh);

    // Lock camera
    LockCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("LockCamera"));
    LockCamera->SetupAttachment(Mesh);
    LockCamera->bAutoActivate = false;

    // Door timeline
    DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
}

void AElectronicLockActor::BeginPlay()
{
    Super::BeginPlay();

    // Store initial transform
    InitialTransform = GetActorTransform();

    // Setup timeline
    if (DoorOpenCurve && DoorTimeline)
    {
        FOnTimelineFloat Progress;
        Progress.BindUFunction(this, FName("HandleDoorProgress"));
        DoorTimeline->AddInterpFloat(DoorOpenCurve, Progress);

        FOnTimelineEvent Finished;
        Finished.BindUFunction(this, FName("OnDoorTimelineFinished"));
        DoorTimeline->SetTimelineFinishedFunc(Finished);
    }
}

void AElectronicLockActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    DoorTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, nullptr);
}

void AElectronicLockActor::Interact(AHorrorGameCharacter* Player)
{
    if (!Player || bIsOpen)
        return;

    PlayerCharacter = Player;
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    // 2) Nếu đã show Note rồi, thì chuyển qua màn hình nhập code
    PC->SetViewTargetWithBlend(this, 0.5f);
    PC->bShowMouseCursor = true;
    Player->DisableInput(PC);

    if (!ElectronicLockWidget && ElectronicLockWidgetClass)
    {
        ElectronicLockWidget = CreateWidget<UElectronicLockWidget>(PC, ElectronicLockWidgetClass);
        ElectronicLockWidget->BindLockActor(this);
        ElectronicLockWidget->AddToViewport();
    }
    else if (ElectronicLockWidget)
    {
        ElectronicLockWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void AElectronicLockActor::EnableMovementPlayer(AHorrorGameCharacter* Player, bool bIsCanceled)
{
    if (!Player) return;
    auto* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    PC->bShowMouseCursor = false;
    PC->SetViewTargetWithBlend(Player, 0.5f);
    Player->EnableInput(PC);

    if (bIsCanceled && ElectronicLockWidget)
    {
        ElectronicLockWidget->RemoveFromParent();
        ElectronicLockWidget = nullptr;
    }
}

void AElectronicLockActor::AddDigit(int32 Digit)
{
    if (bIsOpen) return;

    EnteredCode.Add(Digit);
    OnCodeUpdated.Broadcast(EnteredCode);

    if (EnteredCode.Num() >= CorrectCode.Num())
    {
        VerifyCode();
    }
}

void AElectronicLockActor::VerifyCode()
{
    if (EnteredCode == CorrectCode)
    {
        DoorTimeline->PlayFromStart();
        bIsOpen = true;
    }
    else
    {
        OnCodeError.Broadcast();
        GetWorldTimerManager().SetTimer(ClearCodeHandle, this, &AElectronicLockActor::DelayClearCodeInput, 1.0f, false);
    }
}

void AElectronicLockActor::ClearEnteredCode()
{
    EnteredCode.Empty();
    OnCodeUpdated.Broadcast(EnteredCode);
}

void AElectronicLockActor::DecreaseCode()
{
    if (EnteredCode.Num() > 0)
    {
        EnteredCode.Pop();
        OnCodeUpdated.Broadcast(EnteredCode);
    }
}

void AElectronicLockActor::DelayClearCodeInput()
{
    ClearEnteredCode();
}

void AElectronicLockActor::HandleDoorProgress(float Value)
{
    const float MaxYaw = 90.f;
    FRotator Rot(0.f, Value * MaxYaw, 0.f);
    Door->SetRelativeRotation(Rot);
}

void AElectronicLockActor::OnDoorTimelineFinished()
{
    if (PlayerCharacter)
    {
        auto* PC = UGameplayStatics::GetPlayerController(this, 0);
        PC->bShowMouseCursor = false;
        PC->SetViewTargetWithBlend(PlayerCharacter, 0.5f);
        PlayerCharacter->EnableInput(PC);
    }
    if (ElectronicLockWidget)
    {
        ElectronicLockWidget->RemoveFromParent();
        ElectronicLockWidget = nullptr;
    }

    // Hide item prompt and mesh highlight
    if (ItemWidget)
    {
        ItemWidget->SetVisibility(false);
    }
    if (Mesh)
    {
        Mesh->SetRenderCustomDepth(false);
    }

    // Restore actor transform if needed
    SetActorTransform(InitialTransform);
}
