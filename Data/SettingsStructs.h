#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "SettingsStructs.generated.h"

class UImage;
class USoundCue;

UENUM(BlueprintType)
enum class ENotificationType : uint8
{
    Info        UMETA(DisplayName = "Info"),
    Success     UMETA(DisplayName = "Success"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class ESoundTypes : uint8
{
    TabSwitch       UMETA(DisplayName = "Tab Switch"),
    Hover          UMETA(DisplayName = "Hover"),
    Apply          UMETA(DisplayName = "Apply"),
    Reset          UMETA(DisplayName = "Reset"),
    Back           UMETA(DisplayName = "Back"),
    Error          UMETA(DisplayName = "Error"),
    SettingsChanged UMETA(DisplayName = "Settings Changed")
};

USTRUCT(BlueprintType)
struct HORRORGAME_API FTabInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab Info")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab Info")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab Info")
    UImage* IconWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab Info")
    bool bIsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab Info")
    int32 TabIndex;

    FTabInfo()
        : Name(TEXT(""))
        , Description(TEXT(""))
        , IconWidget(nullptr)
        , bIsEnabled(true)
        , TabIndex(0)
    {
    }

    FTabInfo(const FString& InName, const FString& InDescription, UImage* InIconWidget = nullptr, int32 InTabIndex = 0)
        : Name(InName)
        , Description(InDescription)
        , IconWidget(InIconWidget)
        , bIsEnabled(true)
        , TabIndex(InTabIndex)
    {
    }

    bool IsValid() const
    {
        return !Name.IsEmpty() && !Description.IsEmpty() && TabIndex >= 0;
    }
};

USTRUCT(BlueprintType)
struct HORRORGAME_API FSettingsValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Validation")
    bool bIsValid;

    UPROPERTY(BlueprintReadWrite, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite, Category = "Validation")
    ENotificationType NotificationType;

    FSettingsValidationResult()
        : bIsValid(true)
        , ErrorMessage(TEXT(""))
        , NotificationType(ENotificationType::Info)
    {
    }

    FSettingsValidationResult(bool InIsValid, const FString& InErrorMessage, ENotificationType InType = ENotificationType::Error)
        : bIsValid(InIsValid)
        , ErrorMessage(InErrorMessage)
        , NotificationType(InType)
    {
    }

    static FSettingsValidationResult Success()
    {
        return FSettingsValidationResult(true, TEXT(""), ENotificationType::Success);
    }

    static FSettingsValidationResult Error(const FString& Message)
    {
        return FSettingsValidationResult(false, Message, ENotificationType::Error);
    }
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfirmationResult, bool, bConfirmed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsAppliedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsResetDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuClosedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsValidationDelegate, const FSettingsValidationResult&, ValidationResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabChangedDelegate, int32, NewTab, int32, OldTab);
