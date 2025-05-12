// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "Components/Overlay.h"
#include "InventoryItem.h"
#include "HorrorGame/Widget/ItemDragDropOperation.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "Components/SizeBox.h"

void UInventorySlot::SetSlotContent(UInventoryItem* InventoryItemWidget)
{
    if (ItemContainer)
    {
        ItemContainer->ClearChildren();
        ItemContainer->AddChild(InventoryItemWidget);
    }
}

void UInventorySlot::SetSlotSize(const FVector2D& NewSize)
{
    if (SlotSizeBox)
    {
        SlotSizeBox->SetWidthOverride(NewSize.X);
        SlotSizeBox->SetHeightOverride(NewSize.Y);
    }
}


bool UInventorySlot::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    return (Cast<UItemDragDropOperation>(InOperation) != nullptr);
}

bool UInventorySlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    if (UItemDragDropOperation* Op = Cast<UItemDragDropOperation>(InOperation))
    {
        if (AHorrorGameCharacter* Owner = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn()))
        {
            Owner->SwapInventoryItems(Op->bSourceIsBag, Op->SourceIndex, this->bIsBagSlot, this->SlotIndex);
            return true;
        }
    }
    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
