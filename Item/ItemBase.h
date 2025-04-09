// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HorrorGame/Data/ItemData.h"
#include "ItemBase.generated.h"

UCLASS()
class HORRORGAME_API UItemBase : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FName ID;

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FItemTextData ItemTextData;

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	EItemTypeData ItemTypeData;

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FItemQuantityData ItemQuantityData;	
	
	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FItem3DMeshData Item3DMeshData;

	UFUNCTION(Category = "Item")
	UItemBase* CreateItemCopy() const;

	UFUNCTION(Category = "Item")
	void InitializeFromItemData(const FItemData& Data);

protected:
	bool operator == (const FName& OtherID) const
	{
		return this->ID == OtherID;
	}
};
