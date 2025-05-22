#include "HorrorGame/Actor/NoteActor.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/WidgetAnimation.h"

ANoteActor::ANoteActor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComponent->InitSphereRadius(100.f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetupAttachment(Mesh);

	ItemWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemWidget"));
	ItemWidget->SetupAttachment(SphereComponent);
	ItemWidget->SetWidgetSpace(EWidgetSpace::Screen);
	ItemWidget->SetDrawSize(FVector2D(50, 85));
	ItemWidget->SetVisibility(false);

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ANoteActor::OnOverlapBegin);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &ANoteActor::OnOverlapEnd);
}

void ANoteActor::BeginPlay()
{
	Super::BeginPlay();
	OriginalTransform = GetActorTransform();
}

void ANoteActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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

void ANoteActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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

void ANoteActor::Interact(AActor* Interactor)
{
	// Tell the character to show the note UI
	if (AHorrorGameCharacter* C = Cast<AHorrorGameCharacter>(Interactor))
	{
		C->SetCurrentNoteActor(this);
		HideNote();
		C->ShowNoteUI(NoteImage, NoteText);
	}
}

void ANoteActor::HideNote()
{
	// ẩn mesh + tắt collision
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ANoteActor::ReturnToOriginal()
{
	// trả về vị trí gốc, hiện lại và bật collision
	SetActorTransform(OriginalTransform);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}