#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItem.h" // Forward include to recognize UInventoryItem
#include "InventorySlot.generated.h"

/**
 * Widget for an individual inventory slot, handling drag-and-drop and highlights.
 */
UCLASS()
class HORRORGAME_API UInventorySlot : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Set the content widget (item) inside this slot */
    UFUNCTION(BlueprintCallable, Category = "InventorySlot")
    void SetSlotContent(UInventoryItem* InventoryItemWidget);

    /** Adjust the slot's size via SizeBox overrides */
    UFUNCTION(BlueprintCallable, Category = "InventorySlot")
    void SetSlotSize(const FVector2D& NewSize);

    /** Container for placing the item widget */
    UPROPERTY(meta = (BindWidget))
    class UOverlay* ItemContainer;

    /** SizeBox wrapper for controlling slot dimensions */
    UPROPERTY(meta = (BindWidget))
    class USizeBox* SlotSizeBox;

    /** Index of this slot within inventory or bag */
    UPROPERTY(BlueprintReadOnly)
    int32 SlotIndex;

    /** True if this slot belongs to the auxiliary bag */
    UPROPERTY(BlueprintReadOnly)
    bool bIsBagSlot;

    // Drag-and-drop overrides
    virtual bool NativeOnDragOver(
        const FGeometry& InGeometry,
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

    virtual bool NativeOnDrop(
        const FGeometry& InGeometry,
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

private:

};
