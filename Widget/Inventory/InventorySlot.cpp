// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "Components/Overlay.h"
#include "InventoryItem.h"
#include "HorrorGame/Widget/ItemDragDropOperation.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "Components/SizeBox.h"
#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Widget/Inventory/QuantitySelectionWidget.h"

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

FReply UInventorySlot::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bIsBagSlot)
    {
        // giờ đây bạn đã có actor lưu sẵn
        if (BoundItemActor && BoundItemActor->bIsStackable && BoundItemActor->Quantity > 1)
        {
            if (UQuantitySelectionWidget* Dialog = CreateWidget<UQuantitySelectionWidget>(this, QuantitySelectionClass))
            {
                Dialog->Initialize(SlotIndex, BoundItemActor->Quantity);
                Dialog->OnConfirmed.BindLambda([this](int32 Index, int32 Amount)
                    {
                        OnSplitRequested.Broadcast(Index, Amount);
                    });
                Dialog->AddToViewport();
            }
            return FReply::Handled();
        }
    }
    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}
