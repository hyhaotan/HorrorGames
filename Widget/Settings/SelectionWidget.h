// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CommonButtonBase.h"
#include "SelectionOption.h"
#include "SelectionWidget.generated.h"

class UCommonTextBlock;

UCLASS(Blueprintable,BlueprintType)
class HORRORGAME_API USelectionWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	USelectionWidget();

	void Clear();
	void AddOption(const FSelectionOption& InOption);
	void SetCurrentSelection(int InIndex);

	UFUNCTION(BlueprintCallable)
	void SelectPrevious();

	UFUNCTION(BlueprintCallable)
	void SelectNext();

	DECLARE_DELEGATE_OneParam(FOnSelectionChange, int);
	FOnSelectionChange OnSelectionChange;

protected:
	UFUNCTION()
	UWidget* OnNavigation(EUINavigation InNavigation);

	void UpdateCurrentSelection();

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TArray<FSelectionOption> Options;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Label;

	int CurrentSelection;
};
