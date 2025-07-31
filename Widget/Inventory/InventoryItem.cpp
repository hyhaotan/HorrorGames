// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/InventoryItem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "Components/CanvasPanel.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Item/ItemBase.h"

void UInventoryItem::SetItemImage(UTexture2D* ItemIcon)
{
    if (ItemIconImage)
    {
        if (ItemIcon)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(ItemIcon);
            Brush.ImageSize = FVector2D(100.f, 100.f);
            ItemIconImage->SetBrush(Brush);
        }
        else
        {
            ItemIconImage->SetBrush(FSlateNoResource());
        }
    }
}

void UInventoryItem::SetSlotNumber(int32 SlotNumber)
{
    if (SlotNumberText)
    {
        SlotNumberText->SetText(FText::AsNumber(SlotNumber));
    }
}

void UInventoryItem::SetBoundItem(AItem* InItem)
{
    if (!InItem) return;
    BoundItem = InItem;
    SetItemImage(InItem ? InItem->ItemData->ItemTextData.Icon : nullptr);
}

