// Fill out your copyright notice in the Description page of Project Settings.


#include "ProgressBarWidget.h"
#include "Components/Image.h"

void UProgressBarWidget::SetProgressPercent(float InPercent)
{
    if (EscapeProgressBar)
    {
        EscapeProgressBar->SetPercent(InPercent);
    }
}

void UProgressBarWidget::SetNextKeyImage(UTexture2D* InTexture)
{
    if (NextKeyImage && InTexture)
    {
        // Tạo Slate Brush từ texture rồi apply
        FSlateBrush Brush;
        Brush.SetResourceObject(InTexture);
        // hoặc: NextKeyImage->SetBrushFromTexture(InTexture);
        NextKeyImage->SetBrush(Brush);
    }
}

void UProgressBarWidget::SetPhaseImage(UTexture2D* InTexture)
{
    if (PhaseImage && InTexture)
    {
        // Tạo Slate Brush từ texture rồi apply
        FSlateBrush Brush;
        Brush.SetResourceObject(InTexture);
        // hoặc: NextKeyImage->SetBrushFromTexture(InTexture);
        PhaseImage->SetBrush(Brush);
    }
}

void UProgressBarWidget::PlayPress()
{
    if (PressAnim) PlayAnimation(PressAnim);
}
