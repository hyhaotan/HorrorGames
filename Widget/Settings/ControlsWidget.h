// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "HorrorGame/Data/ControlsData.h"
#include "ControlsWidget.generated.h"

class USlider;
class UTextBlock;
class UCheckBox;
class UScrollBox;

UCLASS()
class HORRORGAME_API UControlsWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
    // Mouse Settings
    UPROPERTY(meta = (BindWidget))
    USlider* MouseSensitivitySlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MouseSensitivityText;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* InvertMouseYCheckBox;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* InvertMouseXCheckBox;

    // Controller Settings
    UPROPERTY(meta = (BindWidget))
    USlider* ControllerSensitivitySlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ControllerSensitivityText;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* ControllerVibrationCheckBox;

    // Key Bindings
    UPROPERTY(meta = (BindWidget))
    UScrollBox* KeyBindingsScrollBox;

    UPROPERTY(meta = (BindWidget))
    UButton* ResetToDefaultsButton;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<class UKeyBindingWidget> KeyBindingWidgetClass;

private:
    void InitializeMouseSettings();
    void InitializeControllerSettings();
    void InitializeKeyBindings();
    void PopulateKeyBindings();

    UFUNCTION()
    void OnMouseSensitivityChanged(float Value);

    UFUNCTION()
    void OnInvertMouseYChanged(bool bIsChecked);

    UFUNCTION()
    void OnInvertMouseXChanged(bool bIsChecked);

    UFUNCTION()
    void OnControllerSensitivityChanged(float Value);

    UFUNCTION()
    void OnControllerVibrationChanged(bool bIsChecked);

    UFUNCTION()
    void OnResetToDefaults();

    TArray<FKeyBinding> DefaultKeyBindings;
    class UHorrorGameSettings* HorrorGameSettings;
};
