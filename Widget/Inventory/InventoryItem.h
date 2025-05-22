#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItem.generated.h"

class AItem;
class UItemInfoWidget;
class UImage;
class UTextBlock;

UCLASS()
class HORRORGAME_API UInventoryItem : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Cài icon từ ngoài vào */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetItemImage(UTexture2D* ItemIcon);

    /** Hiển thị số thứ tự slot */
    UFUNCTION(BlueprintCallable, Category = "InventorySlot")
    void SetSlotNumber(int32 SlotNumber);

    /** Hiển thị số lượng nếu >1 */
    UFUNCTION(BlueprintCallable, Category = "Item Quantity")
    void SetItemQuantity();

    /** Gán item data */
    void SetBoundItem(AItem* InItem);

protected:
    /** Bind widget trong UMG nếu có (ở đây giả sử bạn có blueprint slot chứa Image + TextBlock) */
    UPROPERTY(meta = (BindWidget))
    UImage* ItemIconImage;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SlotNumberText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* QuantityText;

    /** Item logic */
    UPROPERTY()
    AItem* BoundItem = nullptr;

    /** Lưu icon hiện tại */
    UPROPERTY()
    UTexture2D* CurrentIcon = nullptr;

    /** Tooltip widget đang show */
    UPROPERTY()
    UItemInfoWidget* ActiveInfoWidget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UItemInfoWidget> ItemInfoWidgetClass;

    UPROPERTY()
	class UInventoryBagWidget* InventoryBagWidget;

    // ---- overrides để drag/drop + hover ----
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
    virtual FCursorReply NativeOnCursorQuery(const FGeometry& InGeometry,const FPointerEvent& InCursorEvent) override;

private:
    bool bIsDragging = false;
	bool bIsHovering = false;

};
