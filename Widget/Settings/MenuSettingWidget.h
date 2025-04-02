// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MenuSettingWidget.generated.h"

class UButton;
class UMenuSettingSystemWidget;

UCLASS()
class HORRORGAME_API UMenuSettingWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(bindWidget))
	UButton* ResumeButton;

	UPROPERTY(meta=(bindWidget))
	UButton* SettingButton;

	UPROPERTY(meta=(bindWidget))
	UButton* QuitButton;

public:
	void DisplayMenuSetting();
	void DisplayMenu();
	void HiddenMenu();
	void ResumeGame();
	void QuitGame();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Menu | Setting")
	TSubclassOf<UMenuSettingSystemWidget> MenuSettingSystemClass;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Menu | Setting")
	TObjectPtr<UMenuSettingSystemWidget> MenuSettingSystem;
private:
	UPROPERTY(EditDefaultsOnly,Category="Menu | Visible")
	bool bIsVisible;
};
