// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API UInventoryItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetItemImage(UTexture2D* ItemIcon);

    UFUNCTION(BlueprintCallable, Category = "InventorySlot")
    void SetSlotNumber(int32 SlotNumber);

protected:
    UPROPERTY(meta = (BindWidget))
    class UImage* ItemIconImage;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SlotNumberText;
};
