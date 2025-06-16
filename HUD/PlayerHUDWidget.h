// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUDWidget.generated.h"

class UMainHUDWidget;

UCLASS()
class HORRORGAME_API APlayerHUDWidget : public AHUD
{
	GENERATED_BODY()
public:
    virtual void BeginPlay() override;

    UPROPERTY()
    UMainHUDWidget* MainWidgetInstance;
protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UMainHUDWidget> MainWidgetClass;



    // Nếu bạn muốn vẽ crosshair thuần texture:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    UTexture2D* CrosshairTexture;

	virtual void DrawHUD() override;


};
