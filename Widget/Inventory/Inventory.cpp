
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "Components/UniformGridPanel.h"
#include "InventoryItem.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/Actor/Item.h"
#include "InventorySlot.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"

void UInventory::NativeConstruct()
{
    Super::NativeConstruct();

    if (AHorrorGameCharacter* P = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn()))
    {
        // Bind sự kiện mỗi khi equip/unequip
        P->OnItemToggled.AddDynamic(this, &UInventory::HandleItemToggled);
        P->OnInventoryUpdated.AddDynamic(this, &UInventory::HandleInventoryChanged);

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

void UInventory::HandleInventoryChanged(const TArray<AActor*>& NewInventory)
{
    UpdateInventory(NewInventory);
}

void UInventory::UpdateInventory(const TArray<AActor*>& InventoryItems)
{
    if (!InventoryGrid || !InventorySlotClass || !InventoryItemClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("UInventory::UpdateInventory: Missing widget classes or grid"));
        return;
    }

    InventoryGrid->ClearChildren();

    // Lấy character và số ô chứa
    AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn());
    const int32 TotalSlots = Player ? Player->MainInventoryCapacity : 0;

    // Xác định số cột (bạn có thể hardcode hoặc lấy từ biến)
    const int32 NumColumns = 5; // ví dụ 5 cột

    for (int32 SlotIndex = 0; SlotIndex < TotalSlots; ++SlotIndex)
    {
        // Tạo slot widget
        UInventorySlot* SlotWidget = CreateWidget<UInventorySlot>(GetOwningPlayer(), InventorySlotClass);
        if (!SlotWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create InventorySlot at index %d"), SlotIndex);
            continue;
        }

        // Tạo item widget
        UInventoryItem* ItemWidget = CreateWidget<UInventoryItem>(GetOwningPlayer(), InventoryItemClass);
        if (!ItemWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create InventoryItem at index %d"), SlotIndex);
        }

        // Thiết lập slot
        SlotWidget->SlotIndex = SlotIndex;
        SlotWidget->SetHighlight(SlotIndex == CurrentEquippedIndex);
        SlotWidget->bIsBagSlot = false;

        // Thiết lập số thứ tự
        if (ItemWidget)
        {
            ItemWidget->SetSlotNumber(SlotIndex + 1);
        }

        // Lấy item ở slot
        AItem* It = nullptr;
        if (InventoryItems.IsValidIndex(SlotIndex))
        {
            It = Cast<AItem>(InventoryItems[SlotIndex]);
        }

        // Bind item hoặc ẩn widget
        if (ItemWidget)
        {
            if (It)
            {
                // Kiểm tra ItemData
                if (It->ItemData)
                {
                    ItemWidget->SetBoundItem(It);
                    ItemWidget->SetVisibility(ESlateVisibility::Visible);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("ItemData null for %s at slot %d"), *It->GetName(), SlotIndex);
                    ItemWidget->SetVisibility(ESlateVisibility::Collapsed);
                }
            }
            else
            {
                // Không có item, ẩn luôn
                ItemWidget->SetVisibility(ESlateVisibility::Collapsed);
            }

            // Đặt content vào slot
            SlotWidget->SetSlotContent(ItemWidget);
        }

        // Add slot vào grid theo hàng/cột
        const int32 Row = SlotIndex / NumColumns;
        const int32 Column = SlotIndex % NumColumns;
        InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Column);
    }
}
