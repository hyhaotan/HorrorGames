// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KnockOutWidget.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API UKnockOutWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* KnockOutProgressBar;

	virtual void NativeConstruct() override;

	void UpdateKnockOutProgress(float NewProgress);
};
