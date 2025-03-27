
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "Components/UniformGridPanel.h"
#include "InventoryItem.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/Actor/Item.h"
#include "InventorySlot.h"

void UInventory::UpdateInventory(const TArray<AActor*>& InventoryItems)
{
    if (!InventoryGrid || !InventorySlotClass) return;

    // Xóa hết các widget cũ trong UniformGridPanel
    InventoryGrid->ClearChildren();

    const int32 TotalSlots = 3;

    // Tạo 3 slot cố định
    for (int32 SlotIndex = 0; SlotIndex < TotalSlots; SlotIndex++)
    {
        UInventorySlot* SlotWidget = CreateWidget<UInventorySlot>(this, InventorySlotClass);
        UInventoryItem* InventoryItemWidget = CreateWidget<UInventoryItem>(this, InventoryItemClass);
        if (SlotWidget && InventoryItemWidget)
        {
            // Cập nhật số slot (slot bắt đầu từ 1)
            InventoryItemWidget->SetSlotNumber(SlotIndex + 1);

            // Nếu có item tại vị trí SlotIndex, cập nhật hình ảnh
            if (InventoryItems.IsValidIndex(SlotIndex) && InventoryItems[SlotIndex] != nullptr)
            {
                UTexture2D* ItemIcon = nullptr;
                if (AItem* ItemActorObj = Cast<AItem>(InventoryItems[SlotIndex]))
                {
                    if (ItemActorObj->ItemData)
                    {
                        ItemIcon = ItemActorObj->ItemData->ItemTextData.Icon;
                    }
                }
                InventoryItemWidget->SetItemImage(ItemIcon);
            }

            // Luôn luôn thêm InventoryItemWidget vào slot để hiển thị số thứ tự
            SlotWidget->SetSlotContent(InventoryItemWidget);

            // Thêm slot widget vào UniformGridPanel (ví dụ: 1 hàng, 3 cột)
            InventoryGrid->AddChildToUniformGrid(SlotWidget, 0, SlotIndex);
        }
    }
}