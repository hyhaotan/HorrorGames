// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuMode.h"
#include "HorrorGame/Widget/Item/NoteWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"

AMainMenuMode::AMainMenuMode()
{
}

void AMainMenuMode::BeginPlay()
{
    Super::BeginPlay();
    HideAllGameWidgets();
}


void AMainMenuMode::HideAllGameWidgets()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    TArray<UUserWidget*> AllWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        /*WorldContextObject=*/ PC,
        /*OutWidgets=*/ AllWidgets,
        /*WidgetClass=*/ UUserWidget::StaticClass(),
        /*bTopLevelOnly=*/ false
    );

    for (UUserWidget* Widget : AllWidgets)
    {
        if (!Widget || !Widget->IsInViewport()) continue;

        // Giả sử class widget note của bạn là UNoteWidget
        if (Widget->IsA(UNoteWidget::StaticClass()))
        {
            continue;  // không remove note widget
        }

        Widget->RemoveFromParent();
    }
}
