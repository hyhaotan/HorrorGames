#include "HorrorGame/Actor/ElectronicLockActor.h"
#include "HorrorGame/Widget/ElectronicLockWidget.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/WidgetAnimation.h"

AElectronicLockActor::AElectronicLockActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComponent->InitSphereRadius(100.f);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    RootComponent = SphereComponent;

    LockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockMesh"));
    LockMesh->SetupAttachment(RootComponent);
    LockMesh->SetSimulatePhysics(true);
    LockMesh->SetRenderCustomDepth(false);

    ItemWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemWidget"));
    ItemWidget->SetupAttachment(RootComponent);
    ItemWidget->SetWidgetSpace(EWidgetSpace::Screen);
    ItemWidget->SetDrawSize(FVector2D(300, 200));
    ItemWidget->SetVisibility(false);

    LockCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("LockCamera"));
    LockCamera->SetupAttachment(RootComponent);
    LockCamera->SetRelativeLocation(FVector(-200.f, 0.f, 100.f));
    LockCamera->bAutoActivate = false;

    SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AElectronicLockActor::OnOverlapBegin);
    SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AElectronicLockActor::OnOverlapEnd);

    bIsOpen = false;
}

void AElectronicLockActor::BeginPlay()
{
    Super::BeginPlay();
}

void AElectronicLockActor::AddDigit(int32 Digit)
{
    if (bIsOpen) return;

    EnteredCode.Add(Digit);
    OnCodeUpdated.Broadcast(EnteredCode);

    if (EnteredCode.Num() >= CorrectCode.Num())
    {
        if (EnteredCode == CorrectCode)
        {
            OpenDoor();
        }
        else
        {
            EnteredCode.Empty();
            OnCodeError.Broadcast();
        }
    }
}

void AElectronicLockActor::VerifyCode()
{
    // Explicit verify, in case you clear and press enter without full length
    if (EnteredCode == CorrectCode)
    {
        OpenDoor();
    }
    else
    {
        EnteredCode.Empty();
        OnCodeError.Broadcast();
    }
}

void AElectronicLockActor::Interact(AHorrorGameCharacter* Player)
{
    if (!Player) return;

    // Chuyển view target sang camera của ổ khoá
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    PC->SetViewTargetWithBlend(LockCamera->GetOwner(), 0.5f);

    // Khóa input của player
    Player->DisableInput(PC);

    // Tạo và hiển thị widget
    if (!ElectronicLockWidget && ElectronicLockWidgetClass)
    {
        ElectronicLockWidget = CreateWidget<UElectronicLockWidget>(
            PC, ElectronicLockWidgetClass
        );

        ElectronicLockWidget->AddToViewport();
        ElectronicLockWidget->BindLockActor(this);
        PC->SetShowMouseCursor(true);
    }
}

void AElectronicLockActor::UnInteract(AHorrorGameCharacter* Player)
{
    if (!Player || !ElectronicLockWidget) return;

    // Remove widget và restore view target
    ElectronicLockWidget->RemoveFromParent();
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    PC->SetShowMouseCursor(false);
    PC->SetViewTargetWithBlend(Player, 0.5f);

    // Cho phép player control lại
    Player->EnableInput(PC);
}

void AElectronicLockActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor != this && ItemWidget)
    {
        if (AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(OtherActor))
        {
            Player->SetCurrentInteractItem(this);
        }

        ItemWidget->SetVisibility(true);
        LockMesh->SetRenderCustomDepth(true);
        if (UItemWidget* PW = Cast<UItemWidget>(ItemWidget->GetUserWidgetObject()))
        {
            PW->PlayShow();
        }
    }
}


void AElectronicLockActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor != this && ItemWidget)
    {
        if (AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(OtherActor))
        {
            Player->ClearCurrentInteractItem(this);
        }

        if (UItemWidget* PW = Cast<UItemWidget>(ItemWidget->GetUserWidgetObject()))
        {
            FTimerHandle TimerHandle;
            PW->PlayHide();

            if (PW->HideAnim)
            {
                // Lấy end time của animation
                const float HideTime = PW->HideAnim->GetEndTime();

                // Tạo delegate với lambda để ẩn widget
                FTimerDelegate HideDel;
                HideDel.BindLambda([this]()
                    {
                        ItemWidget->SetVisibility(false);
                    });

                // Đặt timer
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, HideDel, HideTime, false);
            }
            else
            {
                // không có animation thì ẩn luôn
                ItemWidget->SetVisibility(false);
            }
        }
    }
}

void AElectronicLockActor::OpenDoor()
{
    if (bIsOpen || !DoorActor) return;
    bIsOpen = true;

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->SetViewTargetWithBlend(this, 0.5f);
    }

    FRotator TargetRot = DoorActor->GetActorRotation() + FRotator(0.f, 90.f, 0.f);
    DoorActor->SetActorRotation(TargetRot);

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->SetViewTargetWithBlend(PC->GetPawn(), 0.5f);
        PC->SetShowMouseCursor(false);
    }

    ItemWidget->SetVisibility(false);
}
