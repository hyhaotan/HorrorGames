// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Actor/InteractableActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
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
    if (AHorrorGameCharacter* MyChar = Cast<AHorrorGameCharacter>(OtherActor))
    {
        MyChar->SetCurrentInteractItem(this);
        Mesh->SetRenderCustomDepth(true);

        ShowWidget(ItemWidget);
    }
}

void AInteractableActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (AHorrorGameCharacter* MyChar = Cast<AHorrorGameCharacter>(OtherActor))
    {
        MyChar->ClearCurrentInteractItem(this);
        Mesh->SetRenderCustomDepth(false);

        HideWidget(ItemWidget);

    }
}

void AInteractableActor::ShowWidget(UWidgetComponent* WidgetComp)
{
    if (UItemWidget* Widget = Cast<UItemWidget>(WidgetComp->GetUserWidgetObject()))
    {
        Widget->PlayShow();
        WidgetComp->SetVisibility(true);
    }
}

void AInteractableActor::HideWidget(UWidgetComponent* WidgetComp)
{
    if (UItemWidget* Widget = Cast<UItemWidget>(WidgetComp->GetUserWidgetObject()))
    {
        Widget->PlayHide();

        if (Widget->HideAnim)
        {
            const float Duration = Widget->HideAnim->GetEndTime();
            FTimerHandle Timer;
            FTimerDelegate Del;
            Del.BindLambda([WidgetComp]() { WidgetComp->SetVisibility(false); });
            GetWorld()->GetTimerManager().SetTimer(Timer, Del, Duration, false);
        }
        else
        {
            WidgetComp->SetVisibility(false);
        }
    }
}

