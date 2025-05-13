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
        ItemWidget->SetBoundItem(ItemActor);
        ItemWidget->SetSlotNumber(Index + 1);
        SlotWidget->SetSlotContent(ItemWidget);
        SlotWidget->BoundItemActor = ItemActor;
        SlotWidget->OnSplitRequested.AddDynamic(this,&UInventoryBagWidget::HandleSplitRequest);

        // Thêm trực tiếp slot đã có SizeBox nội tại vào WrapBox
        if (UWrapBoxSlot* WrapSlot = Cast<UWrapBoxSlot>(BagWrapBox->AddChild(SlotWidget)))
        {
            WrapSlot->SetPadding(FMargin(5));
            WrapSlot->SetHorizontalAlignment(HAlign_Center);
            WrapSlot->SetVerticalAlignment(VAlign_Center);
        }
    }
}

void UInventoryBagWidget::HandleSplitRequest(int32 SlotIndex, int32 SplitAmount)
{
    // Lấy reference Character và mảng InventoryBag
    AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn());
    if (!Player) return;
    TArray<AActor*>& Items = Player->InventoryBag;

    // Lấy item gốc
    AItem* Original = Cast<AItem>(Items[SlotIndex]);
    if (!Original || !Original->bIsStackable || SplitAmount <= 0 || SplitAmount >= Original->Quantity)
        return;

    // Giảm số lượng của stack gốc
    Original->Quantity -= SplitAmount;

    AItem* NewItem = NewObject<AItem>(Player, Original->GetClass());
    NewItem->InitializeFrom(Original, SplitAmount);

    // Tìm slot trống và thêm vào
    int32 EmptyIdx = Items.IndexOfByPredicate([](AActor* A) { return A == nullptr; });
    if (EmptyIdx == INDEX_NONE)
    {
        // rollback số lượng gốc
        Original->Quantity += SplitAmount;
        UE_LOG(LogTemp, Warning, TEXT("Split thất bại: không còn slot trống trong bag"));
    }
    else
    {
        Items[EmptyIdx] = NewItem;
    }

    // Refresh UI
    UpdateBag(Items);
}