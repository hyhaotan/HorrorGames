// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/QuantitySelectionWidget.h"
#include "Components/SpinBox.h"
#include "Components/Button.h"

void UQuantitySelectionWidget::Initialize(int32 InSlotIndex, int32 InMaxQuantity)
{
    SlotIndex = InSlotIndex;
    MaxQuantity = InMaxQuantity;

    // Nếu có thể tách (MaxQuantity > 1), cấu hình spin‑box và bật Confirm
    if (MaxQuantity > 1)
    {
        AmountSpinBox->SetMinValue(1);
        AmountSpinBox->SetMaxValue(MaxQuantity - 1);
        AmountSpinBox->SetValue(1);

        if (ConfirmButton)
        {
            ConfirmButton->SetIsEnabled(true);
            ConfirmButton->SetVisibility(ESlateVisibility::Visible);
        }
        AmountSpinBox->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        if (ConfirmButton)
        {
            ConfirmButton->SetIsEnabled(false);
            ConfirmButton->SetVisibility(ESlateVisibility::Collapsed);
        }
        if (AmountSpinBox)
        {
            AmountSpinBox->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UQuantitySelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Chỉ bind Confirm một lần duy nhất
    if (ConfirmButton && !ConfirmButton->OnClicked.IsAlreadyBound(this, &UQuantitySelectionWidget::OnConfirmClicked))
    {
        ConfirmButton->OnClicked.AddDynamic(this, &UQuantitySelectionWidget::OnConfirmClicked);
    }
}

void UQuantitySelectionWidget::OnConfirmClicked()
{
    if (MaxQuantity <= 1)
    {
        RemoveFromParent();
        return;
    }

    int32 Chosen = FMath::Clamp(
        FMath::RoundToInt(AmountSpinBox->GetValue()),
        1,
        MaxQuantity - 1
    );

    OnConfirmed.ExecuteIfBound(SlotIndex, Chosen);

    RemoveFromParent();
}

