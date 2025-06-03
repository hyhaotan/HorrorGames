// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FriendEntryData.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API UFriendEntryData : public UObject
{
	GENERATED_BODY()
	
public:
	// Hiển thị tên
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	// Dùng FUniqueNetIdRepl để Blueprint cũng có thể lấy
	UPROPERTY(BlueprintReadOnly)
	FUniqueNetIdRepl UniqueIdRepl;

	// String version để bind text
	UPROPERTY(BlueprintReadOnly)
	FString UniqueId;
};
