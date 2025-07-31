// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "Components/Overlay.h"
#include "InventoryItem.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Components/SizeBox.h"
#include "HorrorGame/Actor/Item.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

TWeakObjectPtr<UQuantitySelectionWidget> UInventorySlot::CurrentOpenQuantityDialog = nullptr;

void UInventorySlot::NativeConstruct()
{
}

void UInventorySlot::SetSlotContent(UInventoryItem* InventoryItemWidget)
{
    if (ItemContainer)
    {
        ItemContainer->ClearChildren();
        ItemContainer->AddChild(InventoryItemWidget);
        ContainedItemWidget = InventoryItemWidget;
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
void UInventorySlot::SetHighlight(bool bOn)
{
    if (!SlotBorder) return;

    SlotBorder->SetBrushColor(bOn
        ? FLinearColor(1.f, 0.8f, 0.f, 1.f) 
        : FLinearColor::White
    );
    SlotBorder->SetPadding(bOn
        ? FMargin(4.f)
        : FMargin(2.f)
    );
}
