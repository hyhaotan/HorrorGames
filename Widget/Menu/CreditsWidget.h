// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreditsWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class HORRORGAME_API UCreditsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AudioVisualText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UIUXNarrativeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EngineeringOpsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ArtContentText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExternalServicesMiddlewareText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LegalBusinessText;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

private:
	UFUNCTION()
	void OnCloseCreditsClick();
};
