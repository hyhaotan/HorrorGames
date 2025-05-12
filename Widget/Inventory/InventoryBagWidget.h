#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WrapBox.h"
#include "InventorySlot.h" 
#include "InventoryItem.h"
#include "InventoryBagWidget.generated.h"

UCLASS()
class HORRORGAME_API UInventoryBagWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Cập nhật túi: truyền vào mảng AActor* của InventoryBag
    UFUNCTION(BlueprintCallable, Category = "InventoryBag")
    void UpdateBag(const TArray<AActor*>& BagItems);

protected:
    // WrapBox chứa các slot, tự động xuống dòng
    UPROPERTY(meta = (BindWidget))
    UWrapBox* BagWrapBox;

    // Class của slot để sinh
    UPROPERTY(EditAnywhere, Category = "InventoryBag")
    TSubclassOf<UInventorySlot> InventorySlotClass;  
    
    UPROPERTY(EditAnywhere, Category = "InventoryBag")
    TSubclassOf<UInventoryItem> InventoryItemClass;

    // Kích thước icon slot (px)
    UPROPERTY(EditAnywhere, Category = "InventoryBag")
    FVector2D SlotSize = FVector2D(100.f, 100.f);

    // Thứ tự sắp xếp: 0 = không sort, 1 = theo tên tăng dần, 2 = theo tên giảm dần
    UPROPERTY(EditAnywhere, Category = "InventoryBag")
    int32 SortMode = 0;
};