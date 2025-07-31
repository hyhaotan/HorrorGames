#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItem.h"
#include "InventorySlot.generated.h"

class UQuantitySelectionWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnSplitRequested,
    int32, SlotIndex
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

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnSplitRequested OnSplitRequested;

    UPROPERTY(BlueprintReadOnly)
    bool bIsBagSlot;

    UPROPERTY()
    class AItem* BoundItemActor = nullptr;

    UPROPERTY()
    UInventoryItem* ContainedItemWidget = nullptr;

	virtual void NativeConstruct() override;

private:
    static TWeakObjectPtr<UQuantitySelectionWidget> CurrentOpenQuantityDialog;
};
