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

    UFUNCTION(BlueprintCallable, Category = "InventorySlot")
    void SetSlotNumber(int32 SlotNumber);

    /** Gán item data */
    void SetBoundItem(AItem* InItem);
protected:
    UPROPERTY(meta = (BindWidget))
    UImage* ItemIconImage;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SlotNumberText;

    /** Item logic */
    UPROPERTY()
    AItem* BoundItem = nullptr;

    /** Lưu icon hiện tại */
    UPROPERTY()
    UTexture2D* CurrentIcon = nullptr;

private:
    bool bIsDragging = false;
	bool bIsHovering = false;

};
