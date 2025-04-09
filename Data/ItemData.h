#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.generated.h"

USTRUCT(BlueprintType)
struct FItemTextData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	FText Name;

	UPROPERTY(EditAnywhere)
	int32 price;

	UPROPERTY(EditAnywhere)
	UTexture2D* Icon;
};

USTRUCT(BlueprintType)
struct FItem3DMeshData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh;
};


USTRUCT(BlueprintType)
struct FItemQuantityData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	int32 Weight;
};

UENUM()
enum class EItemTypeData : uint8
{
	Key UMETA(DisplayName = "Key")
};

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FItemTextData ItemTextData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemTypeData ItemTypeData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FItemQuantityData ItemQuantityData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FItem3DMeshData Item3DMeshData;
};