#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "InputCoreTypes.h"
#include "ControlsData.generated.h"

USTRUCT(BlueprintType)
struct FKeyBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ActionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActionTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKey DefaultKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKey CurrentKey;

    FKeyBinding()
    {
        ActionName = FText::GetEmpty();
        ActionTag = NAME_None;
        DefaultKey = EKeys::Invalid;
        CurrentKey = EKeys::Invalid;
    }

    FKeyBinding(const FText& InActionName, FName InActionTag, const FKey& InDefaultKey)
        : ActionName(InActionName), ActionTag(InActionTag), DefaultKey(InDefaultKey), CurrentKey(InDefaultKey)
    {
    }
};