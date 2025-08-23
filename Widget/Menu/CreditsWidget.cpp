// Fill out your copyright notice in the Description page of Project Settings.


#include "CreditsWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

void UCreditsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CloseButton->OnClicked.AddDynamic(this, &UCreditsWidget::OnCloseCreditsClick);
}

void UCreditsWidget::OnCloseCreditsClick()
{
	if (this->IsVisible())
	{
		this->SetVisibility(ESlateVisibility::Hidden);
		this->RemoveFromParent();
	}
}
