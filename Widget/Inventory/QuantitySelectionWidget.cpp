// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/QuantitySelectionWidget.h"
#include "Components/SpinBox.h"
#include "Components/Button.h"

void UQuantitySelectionWidget::Initialize(int32 InSlotIndex, int32 InMaxQuantity)
{
    SlotIndex = InSlotIndex;
    MaxQuantity = InMaxQuantity;

    if (MaxQuantity > 1)
    {
        AmountSpinBox->SetMinValue(1);
        AmountSpinBox->SetMaxValue(MaxQuantity - 1);
        AmountSpinBox->SetValue(1);
    }
    else
    {
        // Disable spinbox hoặc ẩn dialog vì không thể split
    }
}

void UQuantitySelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (ConfirmButton)
    {
        ConfirmButton->OnClicked.AddDynamic(this, &UQuantitySelectionWidget::OnConfirmClicked);
    }
}

void UQuantitySelectionWidget::OnConfirmClicked()
{
    int32 Chosen = FMath::Clamp(FMath::RoundToInt(AmountSpinBox->GetValue()), 1, MaxQuantity - 1);
    OnConfirmed.ExecuteIfBound(SlotIndex, Chosen);
    RemoveFromParent();
}
