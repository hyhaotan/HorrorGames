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
#include "HorrorGame/Widget/Inventory/ItemInfoWidget.h"
#include "HorrorGame/Widget/Inventory/InventoryBagWidget.h"

FReply UInventoryItem::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    SetCursor(EMouseCursor::GrabHand);
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && BoundItem)
    {
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventoryItem::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
    bIsDragging = true;
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
	bIsDragging = false;
    if (UInventorySlot* ParentSlot = GetTypedOuter<UInventorySlot>())
    {
        return ParentSlot->NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
    }
    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UInventoryItem::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	bIsDragging = false;
    if (UItemDragDropOperation* Op = Cast<UItemDragDropOperation>(InOperation))
    {
        if (AHorrorGameCharacter* Owner = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn()))
        {
            Owner->DropInventoryItem(Op->bSourceIsBag, Op->SourceIndex);
        }
    }
}

void UInventoryItem::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    bIsHovering = true;
    if (BoundItem && !ActiveInfoWidget)
    {
        ActiveInfoWidget = CreateWidget<UItemInfoWidget>(GetOwningPlayer(), ItemInfoWidgetClass);
        if (!ActiveInfoWidget) return;

        ActiveInfoWidget->AddToViewport(1000);
        ActiveInfoWidget->InitializeWithItem(BoundItem);

        FVector2D MousePos = FSlateApplication::IsInitialized()
            ? FSlateApplication::Get().GetCursorPos()
            : FVector2D(100, 100);

        const float OffsetX = 16.f;
        const float OffsetY = -180.f;
        ActiveInfoWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
        ActiveInfoWidget->SetPositionInViewport(MousePos + FVector2D(OffsetX, OffsetY), true);
    }
}

void UInventoryItem::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    if (ActiveInfoWidget)
    {
        ActiveInfoWidget->RemoveFromParent();
        ActiveInfoWidget = nullptr;
    }
}

FCursorReply UInventoryItem::NativeOnCursorQuery(const FGeometry& InGeometry, const FPointerEvent& InCursorEvent)
{
    if (bIsDragging) 
    {
        return FCursorReply::Cursor(EMouseCursor::GrabHand);
    }
    else if (bIsHovering)
    {
        return FCursorReply::Cursor(EMouseCursor::Hand);
    }

    return Super::NativeOnCursorQuery(InGeometry, InCursorEvent);
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

