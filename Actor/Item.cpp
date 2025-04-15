// Item.cpp

#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Widget/ItemWidget.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Actor/FireZone.h"
#include "HorrorGame/Actor/GrenadeProjectile.h"
#include "Component/BomComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "Components/PostProcessComponent.h"
#include "Components/TimelineComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AItem::AItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Tạo ItemMesh, đặt làm RootComponent và bật simulate physics
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    RootComponent = ItemMesh;
    if (ItemMesh)
    {
        ItemMesh->SetSimulatePhysics(true);
    }


    // Tạo ItemCollision và attach vào ItemMesh
    ItemCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ItemCollision"));
    ItemCollision->SetupAttachment(ItemMesh);

    // Tạo ItemWidget, attach vào ItemCollision và cấu hình
    ItemWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemWidget"));
    ItemWidget->SetupAttachment(ItemCollision);
    ItemWidget->SetWidgetSpace(EWidgetSpace::Screen);
    ItemWidget->SetVisibility(false);

    // Gán các sự kiện overlap cho ItemCollision
    ItemCollision->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
    ItemCollision->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);

    // Khởi tạo loại item mặc định là Normal
    ItemCategory = EItemCategory::Normal;
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

void AItem::SetItemWeight(int32 Weight)
{
    if (ItemMesh)
    {
        ItemMesh->SetMassOverrideInKg(NAME_None, Weight, true);
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
    // Khi có actor khác va chạm, hiển thị widget chỉ mục
    if (OtherActor && OtherActor != this && ItemWidget)
    {
        ItemWidget->SetVisibility(true);
    }
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    // Khi actor rời vùng collision, ẩn widget chỉ mục
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
            // Tạo một UItemBase mới và khởi tạo từ dữ liệu DataTable
            UItemBase* MyItem = NewObject<UItemBase>(this, UItemBase::StaticClass());
            MyItem->InitializeFromItemData(*DataRow);
            ItemData = MyItem;

            // Cập nhật giao diện widget nếu có
            if (ItemWidget)
            {
                if (UItemWidget* MyWidget = Cast<UItemWidget>(ItemWidget->GetUserWidgetObject()))
                {
                    MyWidget->SetItemData(MyItem->ItemTextData);
                }
            }

            // Gán Static Mesh nếu có
            if (ItemMesh && DataRow->Item3DMeshData.StaticMesh)
            {
                ItemMesh->SetStaticMesh(DataRow->Item3DMeshData.StaticMesh);
                ItemMesh->SetVisibility(true);
            }
            else if (ItemMesh)
            {
                ItemMesh->SetVisibility(false);
            }
        }
    }
}

void AItem::HandleHealthMedicine()
{
    if (AHorrorGameCharacter* Character = Cast<AHorrorGameCharacter>(GetOwner()))
    {
        UE_LOG(LogTemp, Log, TEXT("Using Health Medicine: Increasing health by 25"));
        Character->IncreaseHealth(25.f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Health Medicine usage failed: Owner is not a valid character."));
    }
}

void AItem::HandleStaminaMedicine()
{
    if (AHorrorGameCharacter* Character = Cast<AHorrorGameCharacter>(GetOwner()))
    {
        UE_LOG(LogTemp, Log, TEXT("Using Stamina Medicine: Increasing stamina by 20"));
        Character->IncreaseStamina(20.f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Stamina Medicine usage failed: Owner is not a valid character."));
    }
}

void AItem::HandleUseItem()
{
    // Xử lý sử dụng item tùy theo danh mục loại item
    switch (ItemCategory)
    {
    case EItemCategory::HeathMedicine:
        HandleHealthMedicine();
        break;
    case EItemCategory::StaminaMedicine:
        HandleStaminaMedicine();
        break;
    case EItemCategory::General:
        HandleMolotovCocktail();
        break;
    case EItemCategory::Flash:
        HandleFlashExplosive();
        break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("This item cannot be used."));
        break;
    }
}

void AItem::HandleFlashExplosive()
{
    if (UBomComponent* BomComp = FindComponentByClass<UBomComponent>())
    {
        BomComp->HandleFlashExplosive();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HandleFlashExplosive: BomComponent not found on AItem"));
    }
}

void AItem::HandleMolotovCocktail()
{
    if (UBomComponent* BomComp = FindComponentByClass<UBomComponent>())
    {
        BomComp->HandleMolotovCocktail();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HandleMolotovCocktail: BomComponent not found on AItem"));
    }
}