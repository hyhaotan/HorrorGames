// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Settings/MenuSettingWidget.h"
#include "HorrorGame/GameMode/MainMenuMode.h"
#include "kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "MenuSettingSystemWidget.h"

void UMenuSettingWidget::NativeConstruct()
{
	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &UMenuSettingWidget::ResumeGame);
	}

	if (SettingButton)
	{
		SettingButton->OnClicked.AddDynamic(this, &UMenuSettingWidget::DisplayMenuSetting);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UMenuSettingWidget::QuitGame);
	}
}

void UMenuSettingWidget::DisplayMenuSetting()
{
	if (!MenuSettingSystem && MenuSettingSystemClass)
	{
		MenuSettingSystem = CreateWidget<UMenuSettingSystemWidget>(this, MenuSettingSystemClass);
	}

	if (MenuSettingSystem)
	{
		MenuSettingSystem->AddToViewport();
	}

	this->SetVisibility(ESlateVisibility::Collapsed);
}

void UMenuSettingWidget::DisplayMenu()
{
	// Show menu
	this->SetVisibility(ESlateVisibility::Visible);

	// show mouse pointer and pause game
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(true);
		PlayerController->SetInputMode(FInputModeUIOnly());
	}

	// pause game
	UGameplayStatics::SetGamePaused(this, true);

	bIsVisible = true;
}

void UMenuSettingWidget::HiddenMenu()
{
	// Hidden menu
	this->SetVisibility(ESlateVisibility::Collapsed);

	// Hidden mouse pointer and resume game
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	// Resume game
	UGameplayStatics::SetGamePaused(this, false);

	bIsVisible = false;
}

void UMenuSettingWidget::ResumeGame()
{
	HiddenMenu();
}

void UMenuSettingWidget::QuitGame()
{
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}
