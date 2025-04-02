// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlot.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API UInventorySlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "InventorySlot")
    void SetSlotContent(UInventoryItem* InventoryItemWidget);

protected:
    UPROPERTY(meta = (BindWidget))
    class UOverlay* ItemContainer;                                                                                                                                 
};
