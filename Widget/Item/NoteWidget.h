// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NoteWidget.generated.h"

class UImage;
class UButton;
class UTexture2D;
class UTextBlock;

UCLASS()
class HORRORGAME_API UNoteWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Gọi để thiết lập hình ảnh của note trước khi AddToViewport */
	UFUNCTION(BlueprintCallable, Category = "Note")
	void SetupNote(UTexture2D* InNoteTexture);

	void SetupNoteText(const FText& InNoteText);

protected:
	// Các widget con được bind bằng tên (BindWidget)
	UPROPERTY(meta = (BindWidget))
	UImage* NoteImage;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NoteText;

	virtual void NativeConstruct() override;

	UPROPERTY()
	class AHorrorGameCharacter* OwningCharacter;

private:
	UFUNCTION()
	void OnCloseClicked();
};