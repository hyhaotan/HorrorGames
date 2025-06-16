// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "HorrorGame/Widget/MainHUDWidget.h"
#include "Engine/Canvas.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

void APlayerHUDWidget::BeginPlay()
{
    Super::BeginPlay();

    if (MainWidgetClass)
    {
        MainWidgetInstance = CreateWidget<UMainHUDWidget>(GetWorld(), MainWidgetClass);
        if (MainWidgetInstance)
        {
            MainWidgetInstance->AddToViewport();
            MainWidgetInstance->SetAllPanelsVisibility(true);
		}
    }
}

void APlayerHUDWidget::DrawHUD()
{
    Super::DrawHUD();

    // Ví dụ: vẽ crosshair thuần texture ở giữa màn hình
    if (CrosshairTexture)
    {
        const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
        const FVector2D DrawSize(CrosshairTexture->GetSizeX() * 5.f, CrosshairTexture->GetSizeY() * 5.f);
        const FVector2D DrawPosition = Center - (DrawSize * 0.5f);

        FCanvasTileItem TileItem(DrawPosition, CrosshairTexture->GetResource(), DrawSize, FLinearColor::White);
        TileItem.BlendMode = SE_BLEND_Translucent;
        Canvas->DrawItem(TileItem);
    }
}