#include "HorrorGame/Actor/Door/LockedDoorActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Item/Keys.h"
#include "HorrorGame/Widget/KeyNotificationWidget.h"
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"

ALockedDoorActor::ALockedDoorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Lock mesh
    LockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockMesh"));
    LockMesh->SetupAttachment(DoorMesh);

    // Khởi tạo trạng thái
    RequiredKeyID = NAME_None;
    bIsLocked = true;
}

void ALockedDoorActor::BeginPlay()
{
    Super::BeginPlay();
    LockMesh->SetVisibility(bIsLocked);
}

void ALockedDoorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ALockedDoorActor::OnRep_IsLocked()
{
    LockMesh->SetVisibility(bIsLocked);
}

void ALockedDoorActor::UnlockDoor(AHorrorGameCharacter* Player)
{
    if (Player->EquippedItem)
    {
        if (AKeys* Key = Cast<AKeys>(Player->EquippedItem))
        {
            if (Key->KeyID == RequiredKeyID)
            {
                Player->StoreCurrentHeldObject();
                Key->Destroy();
                Player->Inventory.Remove(Key);
                if (Player->InventoryWidget)
                    Player->InventoryWidget->UpdateInventory(Player->Inventory);
                Player->OnInventoryUpdated.Broadcast(Player->Inventory);
            }
        }
    }
    if (Player->KeyNotificationWidget)
    {
        Player->KeyNotificationWidget->UpdateKeyNotification(TEXT("Mở khóa thành công!"));
    }
}

bool ALockedDoorActor::CanOpenDoor_Implementation(AHorrorGameCharacter* Player)
{
    // Nếu cửa đang khóa, kiểm tra Player có chìa khóa phù hợp không
    if (bIsLocked)
    {
        if (Player && Player->bIsHoldingItem && Player->EquippedItem)
        {
            if (AKeys* Key = Cast<AKeys>(Player->EquippedItem))
            {
                if (Key->KeyID == RequiredKeyID)
                {
                    UnlockDoor(Player);
                    bIsLocked = false;
                    OnRep_IsLocked();
                    return true;
                }
            }
        }
        if (Player && Player->KeyNotificationWidget)
        {
            Player->KeyNotificationWidget->UpdateKeyNotification(RequiredKeyID.ToString());
        }
        return false;
    }
    return true;
}

void ALockedDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALockedDoorActor, bHasOpened);
}
