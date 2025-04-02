// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "Components/Overlay.h"
#include "InventoryItem.h"

void UInventorySlot::SetSlotContent(UInventoryItem* InventoryItemWidget)
{
    if (ItemContainer)
    {
        ItemContainer->ClearChildren();
        ItemContainer->AddChild(InventoryItemWidget);
    }
}