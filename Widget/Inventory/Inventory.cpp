
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "Components/UniformGridPanel.h"
#include "InventoryItem.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/Actor/Item.h"
#include "InventorySlot.h"
#include "HorrorGame/HorrorGameCharacter.h"

void UInventory::NativeConstruct()
{
    Super::NativeConstruct();

    if (AHorrorGameCharacter* P = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn()))
    {
        // Bind sự kiện mỗi khi equip/unequip
        P->OnItemToggled.AddDynamic(this, &UInventory::HandleItemToggled);

        // Lần đầu vẽ UI
        UpdateInventory(P->Inventory);
    }
}

void UInventory::HandleItemToggled(int32 NewEquippedIndex)
{
    CurrentEquippedIndex = NewEquippedIndex;
    if (AHorrorGameCharacter* P = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn()))
    {
        UpdateInventory(P->Inventory);
    }
}

void UInventory::UpdateInventory(const TArray<AActor*>& InventoryItems)
{
    if (!InventoryGrid || !InventorySlotClass || !InventoryItemClass) return;

    InventoryGrid->ClearChildren();

    AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn());
    const int32 TotalSlots = Player ? Player->MainInventoryCapacity : 0;

    for (int32 SlotIndex = 0; SlotIndex < TotalSlots; ++SlotIndex)
    {
        // Tạo slot và item widget
        UInventorySlot* SlotWidget = CreateWidget<UInventorySlot>(this, InventorySlotClass);
        UInventoryItem* ItemWidget = CreateWidget<UInventoryItem>(SlotWidget, InventoryItemClass);

        // Thiết lập index / loại slot
        SlotWidget->SlotIndex = SlotIndex;
        SlotWidget->SetHighlight(SlotIndex == CurrentEquippedIndex);
        SlotWidget->bIsBagSlot = false;
        ItemWidget->SetSlotNumber(SlotIndex + 1);

        // Luôn gán BoundItem, dù nullptr cũng được để widget tự ẩn số lượng
        AItem* It = nullptr;
        if (InventoryItems.IsValidIndex(SlotIndex))
            It = Cast<AItem>(InventoryItems[SlotIndex]);
        ItemWidget->SetBoundItem(It);

        // Đặt content và add vào grid
        SlotWidget->SetSlotContent(ItemWidget);
        InventoryGrid->AddChildToUniformGrid(SlotWidget, 0, SlotIndex);
    }
}
