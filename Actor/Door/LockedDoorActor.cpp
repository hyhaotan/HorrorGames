#include "HorrorGame/Actor/Door/LockedDoorActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Item/Keys.h"
#include "HorrorGame/Widget/KeyNotificationWidget.h"
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ALockedDoorActor::ALockedDoorActor()
{
    // Create lock mesh component
    LockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockMesh"));
    LockMesh->SetupAttachment(DoorMesh);

    // Initialize locked door properties
    RequiredKeyID = NAME_None;
    bIsLocked = true;
    UnlockSuccessMessage = TEXT("Mở khóa thành công!");
}

void ALockedDoorActor::BeginPlay()
{
    Super::BeginPlay();

    // Set lock visibility based on locked state
    if (LockMesh)
    {
        LockMesh->SetVisibility(bIsLocked);
    }
}

void ALockedDoorActor::OnRep_IsLocked()
{
    // Update lock mesh visibility when state replicates
    if (LockMesh)
    {
        LockMesh->SetVisibility(bIsLocked);
    }

    UE_LOG(LogTemp, Log, TEXT("Lock state replicated: %s"),
        bIsLocked ? TEXT("Locked") : TEXT("Unlocked"));
}

bool ALockedDoorActor::CanOpenDoor_Implementation(AHorrorGameCharacter* Player)
{
    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("No player for locked door interaction"));
        return false;
    }

    // If door is animating, don't allow interaction
    if (bIsAnimating)
    {
        UE_LOG(LogTemp, Warning, TEXT("Door is currently animating"));
        return false;
    }

    // If door is unlocked, use parent logic
    if (!bIsLocked)
    {
        return Super::CanOpenDoor_Implementation(Player);
    }

    // Door is locked - check for key
    AKeys* Key = nullptr;
    if (PlayerHasRequiredKey(Player, Key))
    {
        // Player has the right key - unlock the door
        UnlockDoor(Player);
        return true; // Allow door to open after unlocking
    }

    // Player doesn't have key - show notification and deny access
    FString KeyMessage = RequiredKeyID.IsNone() ?
        TEXT("Cần chìa khóa để mở cửa") :
        FString::Printf(TEXT("Cần chìa khóa: %s"), *RequiredKeyID.ToString());

    ShowKeyNotification(Player, KeyMessage);
    return false;
}

void ALockedDoorActor::OnDoorInteraction_Implementation(AHorrorGameCharacter* Player)
{
    // Call parent implementation
    Super::OnDoorInteraction_Implementation(Player);

    // Add locked door specific interaction behavior
    UE_LOG(LogTemp, Log, TEXT("Locked door interaction - Lock State: %s"),
        bIsLocked ? TEXT("Locked") : TEXT("Unlocked"));

    // If door was just unlocked, show success message
    if (!bIsLocked)
    {
        ShowKeyNotification(Player, UnlockSuccessMessage);
    }
}

bool ALockedDoorActor::PlayerHasRequiredKey(AHorrorGameCharacter* Player, AKeys*& OutKey)
{
    OutKey = nullptr;

    if (!Player || !Player->bIsHoldingItem || !Player->EquippedItem)
    {
        return false;
    }

    // Check if equipped item is a key
    AKeys* Key = Cast<AKeys>(Player->EquippedItem);
    if (!Key)
    {
        return false;
    }

    // Check if key ID matches required key
    if (Key->KeyID != RequiredKeyID)
    {
        return false;
    }

    OutKey = Key;
    return true;
}

void ALockedDoorActor::UnlockDoor(AHorrorGameCharacter* Player)
{
    if (!Player)
    {
        return;
    }

    AKeys* Key = nullptr;
    if (!PlayerHasRequiredKey(Player, Key) || !Key)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnlockDoor called but player doesn't have required key"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Unlocking door with key: %s"), *Key->KeyID.ToString());

    // Store the key (put it back in inventory if needed)
    if (Player->bIsHoldingItem)
    {
        Player->StoreCurrentHeldObject();
    }

    // Remove key from inventory and destroy it
    Key->Destroy();
    Player->Inventory.Remove(Key);

    // Update inventory UI
    if (Player->InventoryWidget)
    {
        Player->InventoryWidget->UpdateInventory(Player->Inventory);
    }

    // Broadcast inventory update
    Player->OnInventoryUpdated.Broadcast(Player->Inventory);

    // Unlock the door
    bIsLocked = false;

    // Trigger replication
    OnRep_IsLocked();

    UE_LOG(LogTemp, Log, TEXT("Door successfully unlocked"));
}

void ALockedDoorActor::ShowKeyNotification(AHorrorGameCharacter* Player, const FString& Message)
{
    if (Player && Player->KeyNotificationWidget)
    {
        Player->KeyNotificationWidget->UpdateKeyNotification(Message);
    }
}

void ALockedDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALockedDoorActor, bIsLocked);
}