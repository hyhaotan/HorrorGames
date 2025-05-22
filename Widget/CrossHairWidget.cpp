// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/CrossHairWidget.h"

#include "Components/Image.h"

void UCrossHairWidget::SetCrossHairImage(UTexture2D* CrossHairIcon)
{
    if (!CrossHairImage) return;

    if (CrossHairIcon)
    {
        CrossHairImage->SetBrushFromTexture(CrossHairIcon);
        CrossHairImage->SetCursor(EMouseCursor::Hand);
    }
    else
    {
        CrossHairImage->SetBrushFromTexture(nullptr);
        CrossHairImage->SetCursor(EMouseCursor::Default);
    }
}
