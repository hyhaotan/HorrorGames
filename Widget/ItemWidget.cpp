// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/ItemWidget.h"
#include "Components/TextBlock.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Có thể khởi tạo giao diện ở đây nếu cần
}

void UItemWidget::SetItemData(const FItemTextData& NewData)
{
	if (ItemNameText)
	{
		ItemNameText->SetText(NewData.Name);
	}
}