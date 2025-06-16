// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Actor/InteractableActor.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/WidgetAnimation.h"

// Sets default values
AInteractableActor::AInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComponent->InitSphereRadius(100.f);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetupAttachment(Mesh);

    ItemWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemWidget"));
	ItemWidget->SetupAttachment(Mesh);
    ItemWidget->SetWidgetSpace(EWidgetSpace::Screen);
    ItemWidget->SetDrawSize(FVector2D(50, 85));
    ItemWidget->SetVisibility(false);

    SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AInteractableActor::OnOverlapBegin);
    SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AInteractableActor::OnOverlapEnd);

}

// Called when the game starts or when spawned
void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractableActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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
        Mesh->SetRenderCustomDepth(true);
        if (UItemWidget* PW = Cast<UItemWidget>(ItemWidget->GetUserWidgetObject()))
        {
            PW->PlayShow();
        }
    }
}

void AInteractableActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor != this && ItemWidget)
    {
        if (AHorrorGameCharacter* MyChar = Cast<AHorrorGameCharacter>(OtherActor))
        {
            MyChar->ClearCurrentInteractItem(this);
        }

        Mesh->SetRenderCustomDepth(false);
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

