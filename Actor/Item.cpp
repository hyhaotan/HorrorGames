// Item.cpp

#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Actor/FireZone.h"
#include "HorrorGame/Actor/GrenadeProjectile.h"

#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "Components/PostProcessComponent.h"
#include "Components/TimelineComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Animation/WidgetAnimation.h"


// Sets default values
AItem::AItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Tạo ItemMesh, đặt làm RootComponent và bật simulate physics
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    RootComponent = ItemMesh;
    ItemMesh->SetSimulatePhysics(true);
    ItemMesh->SetRenderCustomDepth(false);
    ItemMesh->SetCustomDepthStencilValue(1);

    // Tạo SphereComponent và attach vào ItemMesh
    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComponent->InitSphereRadius(100.f);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SphereComponent->SetupAttachment(ItemMesh);

    // Tạo ItemWidget, attach vào ItemCollision và cấu hình
    ItemWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemWidget"));
    ItemWidget->SetupAttachment(SphereComponent);
    ItemWidget->SetWidgetSpace(EWidgetSpace::Screen);
    ItemWidget->SetDrawSize(FVector2D(50, 85));
    ItemWidget->SetVisibility(false);

    // Gán các sự kiện overlap cho ItemCollision
    SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
    SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);
}

void AItem::BeginPlay()
{
    Super::BeginPlay();
    InitializeItemData();
}

void AItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AItem::OnPickup()
{
    // Ẩn vật phẩm, vô hiệu collision và tắt simulate physics nếu cần
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    if (ItemMesh && ItemMesh->IsSimulatingPhysics())
    {
        ItemMesh->SetSimulatePhysics(false);
    }
}

void AItem::OnDrop(const FVector& DropLocation)
{
    // Hiển thị lại vật phẩm, bật collision, bật simulate physics và đặt vị trí
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    if (ItemMesh)
    {
        ItemMesh->SetSimulatePhysics(true);
        ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    SetActorLocation(DropLocation);
}

void AItem::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // If non-stackable, enforce single quantity
    if (!bIsStackable)
    {
        Quantity = 1;
        MaxStackSize = 1;
    }
}

void AItem::AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName)
{
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterMesh is null. Cannot attach item."));
        return;
    }

    // Hiển thị vật phẩm, vô hiệu collision và tắt simulate physics nếu đang bật
    SetActorHiddenInGame(false);
    SetActorEnableCollision(false);
    if (ItemMesh && ItemMesh->IsSimulatingPhysics())
    {
        ItemMesh->SetSimulatePhysics(false);
    }

    // Attach vật phẩm vào mesh của nhân vật theo socket chỉ định
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    AttachToComponent(CharacterMesh, AttachRules, SocketName);

    // Cố gắng set Owner cho item thông qua CharacterMesh->GetOwner()
    if (AHorrorGameCharacter* OwnerCharacter = Cast<AHorrorGameCharacter>(CharacterMesh->GetOwner()))
    {
        SetOwner(OwnerCharacter);
        UE_LOG(LogTemp, Log, TEXT("Owner set to: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to set Owner. CharacterMesh->GetOwner() returned null or invalid type."));
    }
}


void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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
        if (UItemWidget* PW = Cast<UItemWidget>(ItemWidget->GetUserWidgetObject()))
        {
            PW->PlayShow();
        }
    }
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor != this && ItemWidget)
    {
        if (AHorrorGameCharacter* MyChar = Cast<AHorrorGameCharacter>(OtherActor))
        {
            MyChar->ClearCurrentInteractItem(this);
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
                GetWorld()->GetTimerManager().SetTimer(TimerHandle,HideDel,HideTime,false);
            }
            else
            {
                // không có animation thì ẩn luôn
                ItemWidget->SetVisibility(false);
            }
        }
    }
}

FItemData* AItem::GetItemData() const
{
    if (!ItemRowHandle.DataTable || ItemRowHandle.RowName.IsNone())
    {
        return nullptr;
    }

    static const FString ContextString(TEXT("Item Data Context"));
    return ItemRowHandle.DataTable->FindRow<FItemData>(ItemRowHandle.RowName, ContextString, true);
}

void AItem::InitializeItemData()
{
    if (FItemData* DataRow = GetItemData())
    {
        ConfigureItemBase(*DataRow);
        ConfigureMesh(*DataRow);
        InitializeStackProperties(*DataRow);
    }
}

void AItem::InitializeFrom(const AItem* Source, int32 InQuantity)
{
    bIsStackable = Source->bIsStackable;
    Quantity = InQuantity;
    MaxStackSize = Source->MaxStackSize;
    ItemRowHandle = Source->ItemRowHandle;
    ItemDataRow = Source->ItemDataRow;
    ItemData = Source->ItemData;
}

void AItem::ConfigureItemBase(const FItemData& DataRow)
{
    UItemBase* MyItem = NewObject<UItemBase>(this, UItemBase::StaticClass());
    MyItem->InitializeFromItemData(DataRow);
    ItemData = MyItem;
}

void AItem::ConfigureMesh(const FItemData& DataRow)
{
    if (ItemMesh && DataRow.Item3DMeshData.StaticMesh)
    {
        ItemMesh->SetStaticMesh(DataRow.Item3DMeshData.StaticMesh);
        ItemMesh->SetVisibility(true);
    }
    else if (ItemMesh)
    {
        ItemMesh->SetVisibility(false);
    }
}

void AItem::InitializeStackProperties(const FItemData& DataRow)
{
    bIsStackable = DataRow.bIsStack;

    Quantity = 1;

    MaxStackSize = bIsStackable? DataRow.MaxStackSize: 1;
}

void AItem::UseItem()
{
    UE_LOG(LogTemp, Warning, TEXT("AItem::UseItem() is not implemented for this item type."));
}