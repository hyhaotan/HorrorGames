// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConfirmExitWidget.generated.h"

class UWidgetAnimation;
class UButton;

UCLASS()
class HORRORGAME_API UConfirmExitWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void ShowAnimExit() { PlayAnimation(FadeInAnimation); }

private:
	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmExitButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelExitButton;

	UPROPERTY(Transient,meta = (BindWidgetAnim))
	UWidgetAnimation* FadeInAnimation;

	UPROPERTY(Transient,meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutAnimation;

	UFUNCTION()
	void OnConfirmExitClicked();

	UFUNCTION()
	void OnCancelExitClicked();

	void HideAnimExit() { PlayAnimation(FadeOutAnimation); }
};
