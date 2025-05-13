// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/InventoryItem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "HorrorGame/Widget/ItemDragDropOperation.h"
#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "Components/CanvasPanel.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Item/ItemBase.h"

FReply UInventoryItem::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && BoundItem)
    {
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventoryItem::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    UItemDragDropOperation* DragOp = NewObject<UItemDragDropOperation>(this);

    if (UInventorySlot* ParentSlot = GetTypedOuter<UInventorySlot>())
    {
        DragOp->SourceIndex = ParentSlot->SlotIndex;
        DragOp->bSourceIsBag = ParentSlot->bIsBagSlot;
    }

    DragOp->DefaultDragVisual = this;
    DragOp->Pivot = EDragPivot::MouseDown;
    OutOperation = DragOp;
}

bool UInventoryItem::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    if (UInventorySlot* ParentSlot = GetTypedOuter<UInventorySlot>())
    {
        return ParentSlot->NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
    }
    return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UInventoryItem::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    if (UInventorySlot* ParentSlot = GetTypedOuter<UInventorySlot>())
    {
        return ParentSlot->NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
    }
    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

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

void UInventoryItem::SetBoundItem(AItem* InItem)
{
    BoundItem = InItem;
    SetItemImage(InItem ? InItem->ItemData->ItemTextData.Icon : nullptr);
    SetItemQuantity();

}

void UInventoryItem::SetSlotNumber(int32 SlotNumber)
{
    if (SlotNumberText)
    {
        SlotNumberText->SetText(FText::AsNumber(SlotNumber));
    }
}

void UInventoryItem::SetItemQuantity()
{
    if (BoundItem && BoundItem->Quantity > 1)
    {
        QuantityText->SetText(FText::AsNumber(BoundItem->Quantity));
        QuantityText->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        QuantityText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UInventoryItem::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

    // Nếu đây là thao tác của item
    if (UItemDragDropOperation* Op = Cast<UItemDragDropOperation>(InOperation))
    {
        // Lấy owner là character của UI
        if (AHorrorGameCharacter* Owner =
            Cast<AHorrorGameCharacter>(GetOwningPlayerPawn()))
        {
            Owner->DropInventoryItem(Op->bSourceIsBag, Op->SourceIndex);
        }
    }
}