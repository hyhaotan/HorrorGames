
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QTEIconRow.generated.h"

//UENUM(BlueprintType)
//enum class EQTEPhase : uint8
//{
//    WASD        UMETA(DisplayName = "WASD"),
//    Arrows      UMETA(DisplayName = "Arrows"),
//    Opposite    UMETA(DisplayName = "OppositePairs")
//};

USTRUCT(BlueprintType)
struct FQTEKeyIconRow : public FTableRowBase
{
    GENERATED_BODY()

    // Tên phím, phải trùng với FKey.GetFName().ToString()
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName KeyName;

    // Icon texture
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct FQTEPhaseIconRow : public FTableRowBase
{
    GENERATED_BODY()

    //UPROPERTY(EditAnywhere, BlueprintReadOnly)
    //EQTEPhase Phase;

    // Icon texture
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> Icon;
};


