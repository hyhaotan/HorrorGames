// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite) int32 SourceIndex;
	UPROPERTY(BlueprintReadWrite) bool bSourceIsBag;

	UPROPERTY(BlueprintReadWrite) class AItem* PayloadItem;
};
