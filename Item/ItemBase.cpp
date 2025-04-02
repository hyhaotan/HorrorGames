// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Item/ItemBase.h"

UItemBase* UItemBase::CreateItemCopy() const
{
	UItemBase* NewItem = NewObject<UItemBase>(GetOuter(), UItemBase::StaticClass());
	NewItem->ID = ID;
	NewItem->ItemTextData = ItemTextData;
	NewItem->ItemTypeData = ItemTypeData;
	NewItem->ItemQuantityData = ItemQuantityData;
	return NewItem;
}

void UItemBase::InitializeFromItemData(const FItemData& Data)
{
	// Gán dữ liệu từ FItemData cho UItemBase
	ID = Data.ID;
	ItemTextData = Data.ItemTextData;
	ItemTypeData = Data.ItemTypeData;
	ItemQuantityData = Data.ItemQuantityData;
}