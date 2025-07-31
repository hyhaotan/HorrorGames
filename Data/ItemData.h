#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "ItemData.generated.h"

USTRUCT(BlueprintType)
struct FItemTextData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	FText Name;

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

UENUM()
enum class EItemTypeData : uint8
{
	Key UMETA(DisplayName = "Key") ,
	Flash UMETA(DisplayName = "Flash"),
	HealthMedicine UMETA(DisplayName = "HealthMedicine"),
	General UMETA(DisplayName = "General"),
	FlashLight UMETA(DisplayName = "FlashLight"),
	None UMETA(DisplayName = "None")
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
	FItem3DMeshData Item3DMeshData;

};

