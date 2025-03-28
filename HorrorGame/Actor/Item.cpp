#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Widget/ItemWidget.h"
#include "HorrorGame/Item/ItemBase.h"

#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = false;

	// Tạo ItemMesh và attach vào RootComponent
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetSimulatePhysics(true);

	// Tạo ItemCollission và attach vào ItemMesh
	ItemCollission = CreateDefaultSubobject<UBoxComponent>(TEXT("ItemCollission"));
	ItemCollission->SetupAttachment(ItemMesh);

	// Tạo ItemWidget và attach vào ItemCollission
	ItemWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemWidget"));
	ItemWidget->SetupAttachment(ItemCollission);
	ItemWidget->SetWidgetSpace(EWidgetSpace::Screen);
	ItemWidget->SetVisibility(false);

	// Gán các sự kiện overlap cho ItemCollission
	ItemCollission->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
	ItemCollission->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	InitializeItemData();
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AItem::OnPickup()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (ItemMesh && ItemMesh->IsSimulatingPhysics())
	{
		ItemMesh->SetSimulatePhysics(false);
	}
}

void AItem::OnDrop(const FVector& DropLocation)
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	if (ItemMesh)
	{
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	SetActorLocation(DropLocation);
}

void AItem::SetItemWeight(int32 Weight)
{
	ItemMesh->SetMassOverrideInKg(NAME_None, Weight, true);
}

void AItem::AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName)
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);
	if (ItemMesh && ItemMesh->IsSimulatingPhysics())
	{
		ItemMesh->SetSimulatePhysics(false);
	}

	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(CharacterMesh, AttachRules, SocketName);
}

void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && ItemWidget)
	{
		ItemWidget->SetVisibility(true);
	}
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this && ItemWidget)
	{
		ItemWidget->SetVisibility(false);
	}
}

void AItem::InitializeItemData()
{
	if (ItemRowHandle.DataTable && !ItemRowHandle.RowName.IsNone())
	{
		static const FString ContextString(TEXT("Item Data Context"));
		FItemData* DataRow = ItemRowHandle.DataTable->FindRow<FItemData>(ItemRowHandle.RowName, ContextString, true);
		if (DataRow)
		{
			UItemBase* MyItem = NewObject<UItemBase>(this, UItemBase::StaticClass());
			MyItem->InitializeFromItemData(*DataRow);

			// Gán dữ liệu cho ItemData của AItem
			ItemData = MyItem;

			// Cập nhật giao diện widget của item (nếu cần)
			if (ItemWidget)
			{
				if (UItemWidget* MyWidget = Cast<UItemWidget>(ItemWidget->GetUserWidgetObject()))
				{
					MyWidget->SetItemData(MyItem->ItemTextData);
				}
			}
		}
	}
}
