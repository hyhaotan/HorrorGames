#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"
#include "HorrorGame/Data/ControlsData.h"
#include "ControlsWidget.generated.h"

// Forward declarations
class USlider;
class UTextBlock;
class UCheckBox;
class UScrollBox;
class UButton;
class UHorrorGameSettings;
class UKeyBindingWidget;

// Enums
UENUM(BlueprintType)
enum class EControlsPreset : uint8
{
    Default         UMETA(DisplayName = "Default"),
    HighSensitivity UMETA(DisplayName = "High Sensitivity"),
    LowSensitivity  UMETA(DisplayName = "Low Sensitivity"),
    Accessibility   UMETA(DisplayName = "Accessibility")
};

UCLASS()
class HORRORGAME_API UControlsWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    // Core overrides
    virtual void NativeConstruct() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    // Public interface functions
    UFUNCTION(BlueprintCallable, Category = "Controls Settings")
    void RefreshValues();

    UFUNCTION(BlueprintCallable, Category = "Controls Settings")
    void ResetToDefaults();

    UFUNCTION(BlueprintCallable, Category = "Controls Settings")
    void SetControlsPreset(EControlsPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Controls Settings")
    void SaveControlsSettings();

    UFUNCTION(BlueprintCallable, Category = "Controls Settings")
    void LoadControlsSettings();

    UFUNCTION(BlueprintCallable, Category = "Controls Settings")
    bool AreControlsAtDefaults();

    UFUNCTION(BlueprintCallable, Category = "Controls Settings")
    void LogCurrentControlsSettings();

protected:
    // === Mouse Settings Widgets ===
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<USlider> MouseSensitivitySlider;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> MouseSensitivityText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCheckBox> InvertMouseYCheckBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCheckBox> InvertMouseXCheckBox;

    // === Controller Settings Widgets ===
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<USlider> ControllerSensitivitySlider;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> ControllerSensitivityText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCheckBox> ControllerVibrationCheckBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<USlider> ControllerDeadZoneSlider;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> ControllerDeadZoneText;

    // === Advanced Settings Widgets ===
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCheckBox> MouseSmoothingCheckBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCheckBox> MouseAccelerationCheckBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCheckBox> RawInputCheckBox;

    // === Key Binding Widgets ===
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UScrollBox> KeyBindingsScrollBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> ResetToDefaultsButton;

    // === Key Binding Widget Class ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Controls Settings")
    TSubclassOf<UKeyBindingWidget> KeyBindingWidgetClass;

    // === Event Handlers ===
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
    void OnControllerDeadZoneChanged(float Value);

    UFUNCTION()
    void OnMouseSmoothingChanged(bool bIsChecked);

    UFUNCTION()
    void OnMouseAccelerationChanged(bool bIsChecked);

    UFUNCTION()
    void OnRawInputChanged(bool bIsChecked);

    UFUNCTION()
    void OnResetToDefaults();

    // === Key Binding Event Handlers ===
    void OnKeyBindingChanged(FName ActionTag, FKey NewKey, bool bIsGamepadKey);
    void OnKeyBindingStarted(FName ActionName);
    void OnKeyBindingCancelled(FName ActionName);
    void OnKeyBindingCompleted(FName ActionName, FKey NewKey);

    // === Data Members ===
    UPROPERTY()
    TObjectPtr<UHorrorGameSettings> HorrorGameSettings;

    UPROPERTY()
    TArray<FKeyBinding> DefaultKeyBindings;

    UPROPERTY()
    TArray<TObjectPtr<UKeyBindingWidget>> KeyBindingWidgets;

    UPROPERTY()
    FName CurrentlyRebindingAction;

private:
    // === Initialization Methods ===
    void InitializeMouseSettings();
    void InitializeControllerSettings();
    void InitializeAdvancedSettings();
    void InitializeKeyBindings();

    // === Key Binding Management ===
    void PopulateKeyBindings();
    void ResetKeyBindingsToDefaults();

    // === Utility Methods ===
    bool ValidateControlsComponents();
    void UpdateSensitivityText(UTextBlock* TextWidget, float Value);

    // === Game Integration ===
    void ApplyMouseSensitivityToPlayer(float Sensitivity);
    void ApplyMouseInversionToPlayer();
    void ApplyControllerSettings();
    void ApplySettingsToGame();

    // === Key Binding Utilities ===
    bool HasKeyConflict(FKey NewKey, FName ExcludeAction, bool bIsGamepadKey);
    void UpdateInputSettings(FName ActionName, FKey Key, bool bIsGamepadKey);
    void ShowKeyConflictDialog(FName ConflictingAction, FKey Key);
    bool IsKeyReserved(FKey Key);

    // === Import/Export ===
    void ExportKeyBindings();
    void ImportKeyBindings(const FString& ImportData);

    // === Testing & Debug ===
    void TestKeyBinding(FName ActionName);
    FString GetActionDisplayName(FName ActionName);
};