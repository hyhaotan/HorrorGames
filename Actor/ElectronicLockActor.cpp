#include "HorrorGame/Actor/ElectronicLockActor.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Widget/ElectronicLockWidget.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TimelineComponent.h"

AElectronicLockActor::AElectronicLockActor()
{
    PrimaryActorTick.bCanEverTick = true;

    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(RootComponent);

    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Door->SetupAttachment(DoorFrame);

    LockCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("LockCamera"));
    LockCamera->SetupAttachment(RootComponent);
    LockCamera->bAutoActivate = false;

    DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
    bTimelineInitialized = false;
}

void AElectronicLockActor::BeginPlay()
{
    Super::BeginPlay();

    if (DoorOpenCurve && DoorTimeline)
    {
        FOnTimelineFloat ProgressFun;
        ProgressFun.BindUFunction(this, FName("HandleDoorProgress"));
        DoorTimeline->AddInterpFloat(DoorOpenCurve, ProgressFun);

        FOnTimelineEvent FinishFun;
        FinishFun.BindUFunction(this, FName("OnDoorTimelineFinished"));
        DoorTimeline->SetTimelineFinishedFunc(FinishFun);
    }
}

void AElectronicLockActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AElectronicLockActor::AddDigit(int32 Digit)
{
    if (bIsOpen)
    {
        return;
    }

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
        OpenDoor(90.0f);
    }
    else
    {
        ElectronicLockWidget->ShowError(); 
        OnCodeError.Broadcast();

        GetWorld()->GetTimerManager().SetTimer(
            ClearCodeHandle, this, &AElectronicLockActor::DelayClearCodeInput, 1.0f, false);
    }
}

void AElectronicLockActor::ClearEnteredCode()
{
    EnteredCode.Empty();
    OnCodeUpdated.Broadcast(EnteredCode);
}

void AElectronicLockActor::DecreseCode()
{
    //Step 1:Use Pop to remove the last digit
    if (EnteredCode.Num() > 0)
    {
        EnteredCode.Pop();
    }

	//step 3: If the last digit is 0, remove it from the array
	//int32 LastIndex = EnteredCode.Num() - 1;
	//if (LastIndex >= 0)
	//{
	//	EnteredCode.RemoveAt(LastIndex);
	//}
    OnCodeUpdated.Broadcast(EnteredCode);
}

void AElectronicLockActor::Interact(AHorrorGameCharacter* Player)
{
    if (!Player || bIsOpen)
    {
        return;
    }

    PlayerCharacter = Player;
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetViewTargetWithBlend(Mesh->GetOwner(), 0.5f);
        PC->SetShowMouseCursor(true);
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
}

void AElectronicLockActor::EnableMovementPlayer(AHorrorGameCharacter* Player, bool bIsCanceled)
{
    if (!Player)
    {
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetShowMouseCursor(false);
        PC->SetViewTargetWithBlend(Player, 0.5f);
        Player->EnableInput(PC);

        if (bIsCanceled && ElectronicLockWidget)
        {
            ElectronicLockWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void AElectronicLockActor::OpenDoor(float DeltaTime)
{
    if (bIsOpen || !DoorTimeline) return;
    bIsOpen = true;

    DoorTimeline->PlayFromStart();
}

void AElectronicLockActor::DelayClearCodeInput()
{
    ClearEnteredCode();
}

void AElectronicLockActor::HandleDoorProgress(float Value)
{
    float TargetYaw = 90.f;
    FRotator R = FRotator(0, Value * TargetYaw, 0);
    Door->SetRelativeRotation(R);
}

void AElectronicLockActor::OnDoorTimelineFinished()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC && PlayerCharacter)
    {
        PC->SetViewTargetWithBlend(PlayerCharacter, 0.5f);
        PC->SetShowMouseCursor(false);
        PlayerCharacter->EnableInput(PC);
    }

    if (ElectronicLockWidget)
    {
        ElectronicLockWidget->RemoveFromParent();
        ElectronicLockWidget = nullptr;
    }
    ItemWidget->SetVisibility(false);
    Mesh->SetRenderCustomDepth(false);
}
