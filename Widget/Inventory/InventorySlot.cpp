// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "Components/Overlay.h"
#include "InventoryItem.h"
#include "HorrorGame/Widget/ItemDragDropOperation.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "Components/SizeBox.h"
#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Widget/Inventory/QuantitySelectionWidget.h"

TWeakObjectPtr<UQuantitySelectionWidget> UInventorySlot::CurrentOpenQuantityDialog = nullptr;

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
    // Only respond to right-click on bag slots
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bIsBagSlot)
    {
        // Close any existing dialog
        if (CurrentOpenQuantityDialog.IsValid())
        {
            CurrentOpenQuantityDialog->RemoveFromParent();
            CurrentOpenQuantityDialog = nullptr;
        }

        // If this slot's dialog is already open, consume event
        if (QuantityDialogWidget && QuantityDialogWidget->IsInViewport())
        {
            QuantityDialogWidget = nullptr;
            return FReply::Handled();
        }

        // Only open when item is stackable and count >1
        if (BoundItemActor && BoundItemActor->bIsStackable && BoundItemActor->Quantity > 1)
        {
            SetQuantitySelectionWidget(InGeometry, InMouseEvent);
            return FReply::Handled();
        }
    }

    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UInventorySlot::SetQuantitySelectionWidget(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // Create and track the dialog
    QuantityDialogWidget = CreateWidget<UQuantitySelectionWidget>(GetOwningPlayer(), QuantitySelectionClass);
    if (!QuantityDialogWidget) return;

    CurrentOpenQuantityDialog = QuantityDialogWidget;

    // Initialize with slot index and max quantity
    QuantityDialogWidget->Initialize(SlotIndex, BoundItemActor->Quantity);

    // Bind confirmation to split logic
    QuantityDialogWidget->OnConfirmed.BindLambda([this](int32 Index, int32 Amount)
        {
            OnSplitRequested.Broadcast(Index, Amount);
            if (CurrentOpenQuantityDialog.IsValid())
            {
                CurrentOpenQuantityDialog->RemoveFromParent();
                CurrentOpenQuantityDialog = nullptr;
            }
            QuantityDialogWidget = nullptr;
        });

    // Position dialog next to slot
    const FVector2D MouseScreenPos = InMouseEvent.GetScreenSpacePosition();
    const FVector2D SlotSizeOnScreen = InGeometry.GetLocalSize() * InGeometry.GetAccumulatedLayoutTransform().GetScale();
    const FVector2D DialogPos = MouseScreenPos + FVector2D(SlotSizeOnScreen.X + 5.f, 0.f);

    QuantityDialogWidget->SetPositionInViewport(DialogPos, true);
    QuantityDialogWidget->AddToViewport();
}