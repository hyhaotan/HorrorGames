// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GraphicsWidget.generated.h"

class UComboBoxString;
class UCheckBox;
class UGameUserSettings;
class USelectionWidget;

UCLASS()
class HORRORGAME_API UGraphicsWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
protected:
	void InitializeResolutionComboBox();
	void InitializeVSync();
	void InitializeFramerate();

	UFUNCTION()
	void OnVSyncChanged(bool InIsChecked);

	UFUNCTION()
	void OnResolutionChanged(FString InSelectedItem, ESelectInfo::Type InSelectionType);

	UPROPERTY()
	TObjectPtr<UGameUserSettings> GameUserSettings;

	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UComboBoxString> ResolutionComboBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> VSyncCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> FramerateSelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> ShadingQualitySelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> GlobalIlluminationQualitySelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> PostProcessingQualitySelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> VisualEffectsQualitySelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> ShadowQualitySelection;

	UPROPERTY()
	TArray<FIntPoint> Resolutions;
};
