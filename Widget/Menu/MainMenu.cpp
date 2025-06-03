// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Menu/MainMenu.h"
#include "HorrorGame/Widget/Menu/GameModeSelection.h"
#include "HorrorGame/Widget/Settings/GraphicsWidget.h"
#include "HorrorGame/Widget/Menu/ConfirmExitWidget.h"

#include "Components/Button.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	PlayButton->OnClicked.AddDynamic(this, &UMainMenu::OnPlayButtonClicked);
	OptionsButton->OnClicked.AddDynamic(this, &UMainMenu::OnOptionsButtonClicked);
	ExitButton->OnClicked.AddDynamic(this, &UMainMenu::OnExitButtonClicked);

}

void UMainMenu::OnPlayButtonClicked()
{
	if (GameModeSelectionClass)
	{
		GameModeSelection = CreateWidget<UGameModeSelection>(GetWorld(), GameModeSelectionClass);
		if (GameModeSelection)
		{
			GameModeSelection->AddToViewport();
			this->RemoveFromViewport();
		}
	}
}

void UMainMenu::OnOptionsButtonClicked()
{
	if (GraphicsWidgetClass)
	{
		GraphicsWidget = CreateWidget<UGraphicsWidget>(GetWorld(), GraphicsWidgetClass);
		if (GraphicsWidget)
		{
			GraphicsWidget->AddToViewport();
		}
	}
}

void UMainMenu::OnExitButtonClicked()
{
    if (!ConfirmExitWidgetClass) return;

    if (!ConfirmExitWidgetInstance)
    {
        ConfirmExitWidgetInstance = CreateWidget<UConfirmExitWidget>(GetWorld(), ConfirmExitWidgetClass);
        if (ConfirmExitWidgetInstance)
        {
			ConfirmExitWidgetInstance->ShowAnimExit();
            ConfirmExitWidgetInstance->AddToViewport(100);
        }
    }
	else
	{
		ConfirmExitWidgetInstance->ShowAnimExit(); 
		ConfirmExitWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	}
}
