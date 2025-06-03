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
    if (Mesh && Mesh->IsSimulatingPhysics())
    {
        Mesh->SetSimulatePhysics(false);
    }
}

void AItem::OnDrop(const FVector& DropLocation)
{
    // Hiển thị lại vật phẩm, bật collision, bật simulate physics và đặt vị trí
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    if (Mesh)
    {
        Mesh->SetSimulatePhysics(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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
    if (Mesh && Mesh->IsSimulatingPhysics())
    {
        Mesh->SetSimulatePhysics(false);
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
    if (Mesh && DataRow.Item3DMeshData.StaticMesh)
    {
        Mesh->SetStaticMesh(DataRow.Item3DMeshData.StaticMesh);
        Mesh->SetVisibility(true);
    }
    else if (Mesh)
    {
        Mesh->SetVisibility(false);
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