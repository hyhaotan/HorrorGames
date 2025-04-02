
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory.generated.h"

UCLASS()
class HORRORGAME_API UInventory : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UpdateInventory(const TArray<AActor*>& InventoryItems);

protected:
    UPROPERTY(meta = (BindWidget))
    class UUniformGridPanel* InventoryGrid;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<class UInventorySlot> InventorySlotClass;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<class UInventoryItem> InventoryItemClass;
};
