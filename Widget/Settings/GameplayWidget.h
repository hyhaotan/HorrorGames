#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayWidget.generated.h"

UCLASS()
class HORRORGAME_API UGameplayWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
    // Difficulty Settings
    UPROPERTY(meta = (BindWidget))
    class USelectionWidget* DifficultySelection;

    // Gameplay Options
    UPROPERTY(meta = (BindWidget))
    class UCheckBox* SubtitlesCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* AutoSaveCheckBox;

    UPROPERTY(meta = (BindWidget))
    class USelectionWidget* LanguageSelection;

    UPROPERTY(meta = (BindWidget))
    class USlider* UIScaleSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* UIScaleValueText;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* TutorialHintsCheckBox;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* CameraShakeCheckBox;

    UPROPERTY(meta = (BindWidget))
    class USlider* InteractionHoldTimeSlider;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* InteractionHoldTimeText;

private:
    void InitializeDifficulty();
    void InitializeLanguage();
    void InitializeGameplayOptions();

    UFUNCTION()
    void OnSubtitlesChanged(bool bIsChecked);

    UFUNCTION()
    void OnAutoSaveChanged(bool bIsChecked);

    UFUNCTION()
    void OnUIScaleChanged(float Value);

    UFUNCTION()
    void OnTutorialHintsChanged(bool bIsChecked);

    UFUNCTION()
    void OnCameraShakeChanged(bool bIsChecked);

    UFUNCTION()
    void OnInteractionHoldTimeChanged(float Value);

    class UGameUserSettings* GameUserSettings;
    class UHorrorGameSettings* HorrorGameSettings;
};