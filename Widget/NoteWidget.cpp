// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/NoteWidget.h"
#include "HorrorGame/HorrorGameCharacter.h"

#include "Components/Image.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h" 
#include "Components/TextBlock.h"

void UNoteWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UNoteWidget::OnCloseClicked);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		OwningCharacter = Cast<AHorrorGameCharacter>(PC->GetPawn());
	}
}

void UNoteWidget::SetupNote(UTexture2D* InNoteTexture)
{
	if (NoteImage && InNoteTexture)
	{
		NoteImage->SetBrushFromTexture(InNoteTexture);
	}
}

void UNoteWidget::SetupNoteText(const FText& InNoteText)
{
	if (NoteText)
	{
		NoteText->SetText(InNoteText);
	}
}

void UNoteWidget::OnCloseClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->bShowMouseCursor = false;
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
	}

	// Notify the character to restore the note actor
	if (OwningCharacter)
	{
		OwningCharacter->CloseNoteUI();
	}
}