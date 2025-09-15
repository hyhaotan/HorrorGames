#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "InputCoreTypes.h"
#include "ControlsData.generated.h"

USTRUCT(BlueprintType)
struct FKeyBinding : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText ActionName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ActionTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FKey DefaultKey;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FKey DefaultGamepadKey;

    UPROPERTY(BlueprintReadOnly)
    FKey CurrentKey;

    UPROPERTY(BlueprintReadOnly)
    FKey CurrentGamepadKey;

    FKeyBinding()
    {
        ActionName = FText::GetEmpty();
        ActionTag = NAME_None;
        DefaultKey = EKeys::Invalid;
        DefaultGamepadKey = EKeys::Invalid;
        CurrentKey = EKeys::Invalid;
        CurrentGamepadKey = EKeys::Invalid;
    }

    FKeyBinding(const FText& InDisplayName, const FName& InActionTag, const FKey& InDefaultKey, const FKey& InDefaultGamepadKey = EKeys::Invalid)
        : ActionName(InDisplayName)
        , ActionTag(InActionTag)
        , DefaultKey(InDefaultKey)
        , DefaultGamepadKey(InDefaultGamepadKey)
        , CurrentKey(InDefaultKey)
        , CurrentGamepadKey(InDefaultGamepadKey)
    {
    }
};