#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/ComboBoxString.h"
#include "GraphicsWidget.generated.h"

// Forward declarations
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
	// Core overrides
	virtual void NativeConstruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// Public interface functions
	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void RefreshValues();

	UFUNCTION(BlueprintCallable, Category = "Graphics Settings")
	void ResetToDefaults();

protected:
	// === Core Widget Bindings ===
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ResolutionComboBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> VSyncCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> FramerateSelection;

	// === Quality Selection Widgets ===
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

	// === Advanced Quality Selection Widgets ===
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> TextureQualitySelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> ViewDistanceQualitySelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> AntiAliasingQualitySelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> ReflectionQualitySelection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> FoliageQualitySelection;

	// === Advanced Graphics Options ===
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> MotionBlurCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> BloomCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> LensFlareCheckBox;

	// === Field of View Controls ===
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> FieldOfViewSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> FieldOfViewText;

	// === Display Options ===
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectionWidget> FullscreenModeSelection;

	// === Action Buttons ===
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> BenchmarkButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> AutoDetectButton;

	// === Core Initialization Methods ===
	void InitializeResolutionComboBox();
	void InitializeVSync();
	void InitializeFramerate();
	void InitializeQualitySettings();

	// === Event Handlers ===
	UFUNCTION()
	void OnVSyncChanged(bool InIsChecked);

	UFUNCTION()
	void OnResolutionChanged(FString InSelectedItem, ESelectInfo::Type InSelectionType);

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

	UFUNCTION()
	void OnAutoDetectClicked();

	// === Data Members ===
	UPROPERTY()
	TObjectPtr<UGameUserSettings> GameUserSettings;

	UPROPERTY()
	TArray<FIntPoint> Resolutions;

private:
	// === Advanced Initialization Methods ===
	void InitializeAdvancedGraphicsOptions();
	void InitializeDisplayOptions();

	// === Refresh Methods ===
	void RefreshQualitySelections();
	void RefreshAdvancedOptions();
	void RefreshDisplayOptions();

	// === Utility Methods ===
	void ResetCustomGraphicsSettings();
	int32 FindClosestResolutionIndex(const FIntPoint& TargetResolution);
	bool ValidateGraphicsComponents();

	// === Settings Management ===
	void ApplySettingsWithValidation();
	void SaveUserSettings();
	void LoadUserSettings();

	// === Horror Game Specific ===
	void SetHorrorGameDefaults();
	void ResetConsoleVariables();
	void OptimizeForHorrorGame();

	// === Quality Management ===
	void SetPresetQuality(int32 QualityPreset);
	FString GetQualityLevelName(int32 QualityLevel);

	// === Resolution Management ===
	bool IsResolutionSupported(const FIntPoint& Resolution);
	void RefreshResolutionList();
	TArray<FIntPoint> GetSupportedResolutions() const;

	// === Debug & Utility ===
	void LogCurrentSettings();
	void OnWindowModeChanged();

	// === Additional Initialization ===
	void InitializeFieldOfView();
	void InitializeActionButtons();
};