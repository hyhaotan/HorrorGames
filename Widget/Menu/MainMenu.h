// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

class UButton;
class UGameModeSelection;
class UGraphicsWidget;
class UConfirmExitWidget;

UCLASS()
class HORRORGAME_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UGameModeSelection* GameModeSelection;

	UPROPERTY()
	UGraphicsWidget* GraphicsWidget;

	UPROPERTY(EditAnywhere, Category = "Menu")
	TSubclassOf<UGameModeSelection> GameModeSelectionClass;

	UPROPERTY(EditAnywhere, Category = "Menu")
	TSubclassOf<class UGraphicsWidget> GraphicsWidgetClass;

	UPROPERTY()
	UConfirmExitWidget* ConfirmExitWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UConfirmExitWidget> ConfirmExitWidgetClass;

private:
	//=================VARIABLES=========================
	UPROPERTY(meta = (BindWidget))
	UButton* PlayButton;
	UPROPERTY(meta = (BindWidget))
	UButton* OptionsButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	//=================FUNCTIONS=========================
	
	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnOptionsButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	virtual void NativeConstruct() override;
};
