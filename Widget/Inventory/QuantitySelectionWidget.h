// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuantitySelectionWidget.generated.h"

UCLASS()
class HORRORGAME_API UQuantitySelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    DECLARE_DELEGATE_TwoParams(FOnConfirmed, int32, int32);
    FOnConfirmed OnConfirmed;

    void Initialize(int32 InSlotIndex, int32 InMaxQuantity);

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnConfirmClicked();

    UPROPERTY(meta = (BindWidget))
    class USpinBox* AmountSpinBox;

    UPROPERTY(meta = (BindWidget))
    class UButton* ConfirmButton;

    int32 SlotIndex;
    int32 MaxQuantity;
};
