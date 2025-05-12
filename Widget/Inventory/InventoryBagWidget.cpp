// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Inventory/InventoryBagWidget.h"
#include "Components/WrapBoxSlot.h"
#include "HorrorGame/Widget/Inventory/InventoryItem.h"
#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Item/ItemBase.h"
#include "Components/WrapBox.h"
#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"
#include "HorrorGame/HorrorGameCharacter.h"

void UInventoryBagWidget::UpdateBag(const TArray<AActor*>& BagItems)
{
    if (!BagWrapBox || !InventorySlotClass || !InventoryItemClass)
        return;

    BagWrapBox->ClearChildren();
    AHorrorGameCharacter* PlayerCharacter = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn());
    if (!PlayerCharacter)
        return;

    const int32 BagCapacity = PlayerCharacter->BagCapacity;

    for (int32 Index = 0; Index < BagCapacity; ++Index)
    {
        // Tạo slot widget và set kích thước ngay bên trong
        UInventorySlot* SlotWidget = CreateWidget<UInventorySlot>(this, InventorySlotClass);
        SlotWidget->SlotIndex = Index;
        SlotWidget->bIsBagSlot = true;
        SlotWidget->SetSlotSize(SlotSize);

        // Tạo item widget và gán hình / số
        UInventoryItem* ItemWidget = CreateWidget<UInventoryItem>(SlotWidget, InventoryItemClass);
        AItem* ItemActor = (BagItems.IsValidIndex(Index) ? Cast<AItem>(BagItems[Index]) : nullptr);
        ItemWidget->SetItemImage(ItemActor ? ItemActor->ItemData->ItemTextData.Icon : nullptr);
        ItemWidget->SetSlotNumber(Index + 1);
        SlotWidget->SetSlotContent(ItemWidget);

        // Thêm trực tiếp slot đã có SizeBox nội tại vào WrapBox
        if (UWrapBoxSlot* WrapSlot = Cast<UWrapBoxSlot>(BagWrapBox->AddChild(SlotWidget)))
        {
            WrapSlot->SetPadding(FMargin(5));
            WrapSlot->SetHorizontalAlignment(HAlign_Center);
            WrapSlot->SetVerticalAlignment(VAlign_Center);
        }
    }
}
