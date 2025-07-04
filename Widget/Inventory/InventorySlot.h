#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItem.h"
#include "InventorySlot.generated.h"

class UQuantitySelectionWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnSplitRequested,
    int32, SlotIndex,
    int32, Amount
);

UCLASS()
class HORRORGAME_API UInventorySlot : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 SlotIndex = INDEX_NONE;

    UFUNCTION(BlueprintCallable, Category = "InventorySlot")
    void SetSlotContent(UInventoryItem* InventoryItemWidget);

    UFUNCTION(BlueprintCallable, Category = "InventorySlot")
    void SetSlotSize(const FVector2D& NewSize);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetHighlight(bool bOn);

    UPROPERTY(meta = (BindWidget))
    class UOverlay* ItemContainer;

    UPROPERTY(meta = (BindWidget))
    class USizeBox* SlotSizeBox;

    UPROPERTY(meta = (BindWidget))
    class UBorder* SlotBorder;

    UPROPERTY(BlueprintReadOnly)
    bool bIsBagSlot;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnSplitRequested OnSplitRequested;

    UPROPERTY()
    class AItem* BoundItemActor = nullptr;

    UPROPERTY()
    UInventoryItem* ContainedItemWidget = nullptr;

	virtual void NativeConstruct() override;
    virtual bool NativeOnDragOver( const FGeometry& InGeometry,const FDragDropEvent& InDragDropEvent,UDragDropOperation* InOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry,const FDragDropEvent& InDragDropEvent,UDragDropOperation* InOperation) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
    UPROPERTY()
    UQuantitySelectionWidget* QuantityDialogWidget = nullptr;

    UFUNCTION()
    void SetQuantitySelectionWidget(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);

    void HandleQuantityChanged(int32 SlotsIndex);
    void HandleItemToggled(int32 SlotsIndex);

    static TWeakObjectPtr<UQuantitySelectionWidget> CurrentOpenQuantityDialog;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UQuantitySelectionWidget> QuantitySelectionClass;
};
