#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItem.generated.h"

UCLASS()
class HORRORGAME_API UInventoryItem : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetItemImage(UTexture2D* ItemIcon);

    UFUNCTION(BlueprintCallable, Category = "InventorySlot")
    void SetSlotNumber(int32 SlotNumber);

    UFUNCTION(BlueprintCallable)
    UTexture2D* GetCurrentIcon() const { return CurrentIcon; }

protected:
    // Widget bindings
    UPROPERTY(meta = (BindWidget))
    class UImage* ItemIconImage;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SlotNumberText;

    // MỚI: biến để lưu icon hiện tại
    UPROPERTY()
    UTexture2D* CurrentIcon;

    // Overrides
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void    NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool    NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool    NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,UDragDropOperation* InOperation) override;
};
