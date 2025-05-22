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

    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
    Door->SetupAttachment(DoorFrame);

    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComponent->InitSphereRadius(100.f);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SphereComponent->SetupAttachment(DoorFrame);

    ItemWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemWidget"));
    ItemWidget->SetupAttachment(SphereComponent);
    ItemWidget->SetWidgetSpace(EWidgetSpace::Screen);
    ItemWidget->SetDrawSize(FVector2D(50, 85));
    ItemWidget->SetVisibility(false);

    SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ADoor::OnOverlapBegin);
    SphereComponent->OnComponentEndOverlap.AddDynamic(this, &ADoor::OnOverlapEnd);

    // Default values
    DoorRotateAngle = 90.f;
    bIsDoorClosed = true;
    bDoorOnSameSide = false;
    Player = nullptr;
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

void ADoor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor != this && ItemWidget)
    {
        if (AHorrorGameCharacter* MyChar = Cast<AHorrorGameCharacter>(OtherActor))
        {
            MyChar->SetCurrentInteractItem(this);
        }

        ItemWidget->SetVisibility(true);
        DoorFrame->SetRenderCustomDepth(true);
        if (UItemWidget* PW = Cast<UItemWidget>(ItemWidget->GetUserWidgetObject()))
        {
            PW->PlayShow();
        }
    }
}

void ADoor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor != this && ItemWidget)
    {
        if (AHorrorGameCharacter* MyChar = Cast<AHorrorGameCharacter>(OtherActor))
        {
            MyChar->ClearCurrentInteractItem(this);
        }

        DoorFrame->SetRenderCustomDepth(false);
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
                GetWorld()->GetTimerManager().SetTimer(
                    /*out*/ TimerHandle,
                    HideDel,
                    HideTime,
                    false
                );
            }
            else
            {
                // không có animation thì ẩn luôn
                ItemWidget->SetVisibility(false);
            }
        }
    }
}

void ADoor::Interact()
{
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
    Door->SetRelativeRotation(FRotator(0.f, Angle * Value, 0.f));
}

void ADoor::SetDoorSameSide()
{
    if (!Player) return;

    FVector DoorToPlayer = Player->GetActorLocation() - GetActorLocation();
    FVector DoorForward = GetActorForwardVector();

    bDoorOnSameSide = FVector::DotProduct(DoorToPlayer, DoorForward) >= 0;
}
