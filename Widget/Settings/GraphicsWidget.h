
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GraphicsWidget.generated.h"

class UComboBoxString;
class UCheckBox;
class UGameUserSettings;
class USelectionWidget;
class UButton;
class USlider;
class UTextBlock;

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

    UPROPERTY(meta = (BindWidget))
    USelectionWidget* TextureQualitySelection;

    UPROPERTY(meta = (BindWidget))
    USelectionWidget* ViewDistanceQualitySelection;

    UPROPERTY(meta = (BindWidget))
    USelectionWidget* AntiAliasingQualitySelection;

    UPROPERTY(meta = (BindWidget))
    USelectionWidget* ReflectionQualitySelection;

    UPROPERTY(meta = (BindWidget))
    USelectionWidget* FoliageQualitySelection;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* MotionBlurCheckBox;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* BloomCheckBox;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* LensFlareCheckBox;

    UPROPERTY(meta = (BindWidget))
    USlider* FieldOfViewSlider;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* FieldOfViewText;

    UPROPERTY(meta = (BindWidget))
    USelectionWidget* FullscreenModeSelection;

    UPROPERTY(meta = (BindWidget))
    UButton* BenchmarkButton;

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

private:
    void InitializeAdvancedGraphicsOptions();
    void InitializeDisplayOptions();

    UFUNCTION()
    void OnMotionBlurChanged(bool bIsChecked);

    UFUNCTION()
    void OnBloomChanged(bool bIsChecked);

    UFUNCTION()
    void OnLensFlareChanged(bool bIsChecked);

    UFUNCTION()
    void OnFieldOfViewChanged(float Value);

    UFUNCTION()
    void OnBenchmarkClicked();
};
