
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "Components/UniformGridPanel.h"
#include "InventoryItem.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/Actor/Item.h"
#include "InventorySlot.h"
#include "HorrorGame/HorrorGameCharacter.h"

void UInventory::UpdateInventory(const TArray<AActor*>& InventoryItems)
{
    if (!InventoryGrid || !InventorySlotClass || !InventoryItemClass) return;

    InventoryGrid->ClearChildren();

    // Lấy capacity từ character, không hard‑code
    AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn());
    const int32 TotalSlots = Player ? Player->MainInventoryCapacity : 0;

    for (int32 SlotIndex = 0; SlotIndex < TotalSlots; ++SlotIndex)
    {
        UInventorySlot* SlotWidget = CreateWidget<UInventorySlot>(this, InventorySlotClass);
        UInventoryItem* ItemWidget = CreateWidget<UInventoryItem>(SlotWidget, InventoryItemClass);
        SlotWidget->SlotIndex = SlotIndex;
        SlotWidget->bIsBagSlot = false;

        // set số, hình giống trước
        ItemWidget->SetSlotNumber(SlotIndex + 1);
        if (InventoryItems.IsValidIndex(SlotIndex) && InventoryItems[SlotIndex])
        {
            if (AItem* It = Cast<AItem>(InventoryItems[SlotIndex]))
                ItemWidget->SetBoundItem(It);
        }
        SlotWidget->SetSlotContent(ItemWidget);
        InventoryGrid->AddChildToUniformGrid(SlotWidget, 0, SlotIndex);
    }
}