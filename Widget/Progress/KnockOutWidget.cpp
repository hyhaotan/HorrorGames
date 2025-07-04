// Fill out your copyright notice in the Description page of Project Settings.


#include "KnockOutWidget.h"
#include "HorrorGame/HorrorGameCharacter.h"

#include "Components/ProgressBar.h"
#include "GameFramework/CharacterMovementComponent.h"

void UKnockOutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (KnockOutProgressBar)
	{
		KnockOutProgressBar->SetPercent(1.0f);
		KnockOutProgressBar->SetFillColorAndOpacity(FLinearColor::Red);
	}
}

void UKnockOutWidget::UpdateKnockOutProgress(float NewProgress)
{
	if (KnockOutProgressBar)
	{
		KnockOutProgressBar->SetPercent(NewProgress);
	}
}
