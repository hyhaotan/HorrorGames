// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrossHairWidget.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API UCrossHairWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetCrossHairImage(UTexture2D* CrossHairIcon);
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* CrossHairImage;


};
