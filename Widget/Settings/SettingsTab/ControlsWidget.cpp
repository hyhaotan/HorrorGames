#include "ControlsWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Engine/World.h"
#include "HorrorGame/Controller/SettingsController.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "HorrorGame/Widget/Settings/KeyBindingWidget.h"

namespace ControlsWidgetPrivate
{
    // Default control values
    static constexpr float DefaultMouseSensitivity = 1.0f;
    static constexpr float DefaultControllerSensitivity = 1.0f;
    static constexpr float DefaultControllerDeadZone = 0.25f;
    static constexpr float MinSensitivity = 0.1f;
    static constexpr float MaxSensitivity = 5.0f;
    static constexpr float MinDeadZone = 0.0f;
    static constexpr float MaxDeadZone = 0.9f;

    // Sensitivity step size for smooth adjustment
    static constexpr float SensitivityStep = 0.1f;
    static constexpr float DeadZoneStep = 0.05f;
}

void UControlsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize horror game settings
    HorrorGameSettings = UHorrorGameSettings::Get();
    if (!HorrorGameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("ControlsWidget: Failed to get HorrorGameSettings"));
        return;
    }

    // Validate all required UI components
    if (!ValidateControlsComponents())
    {
        UE_LOG(LogTemp, Error, TEXT("ControlsWidget: Component validation failed"));
        return;
    }

    // Initialize all control settings
    InitializeMouseSettings();
    InitializeControllerSettings();
    InitializeAdvancedSettings();
    InitializeKeyBindings();

    UE_LOG(LogTemp, Log, TEXT("ControlsWidget: Successfully initialized"));
}

UWidget* UControlsWidget::NativeGetDesiredFocusTarget() const
{
    return MouseSensitivitySlider ? MouseSensitivitySlider : nullptr;
}

void UControlsWidget::InitializeMouseSettings()
{
    if (!HorrorGameSettings)
        return;

    // Initialize Mouse Sensitivity
    if (MouseSensitivitySlider && MouseSensitivityText)
    {
        const float MouseSens = HorrorGameSettings->GetMouseSensitivity();

        // Configure slider properties
        MouseSensitivitySlider->SetMinValue(ControlsWidgetPrivate::MinSensitivity);
        MouseSensitivitySlider->SetMaxValue(ControlsWidgetPrivate::MaxSensitivity);
        MouseSensitivitySlider->SetStepSize(ControlsWidgetPrivate::SensitivityStep);
        MouseSensitivitySlider->SetValue(MouseSens);

        // Update text display
        UpdateSensitivityText(MouseSensitivityText, MouseSens);

        // Bind event handler
        MouseSensitivitySlider->OnValueChanged.Clear();
        MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UControlsWidget::OnMouseSensitivityChanged);

        UE_LOG(LogTemp, Log, TEXT("Mouse sensitivity initialized: %.2f"), MouseSens);
    }

    // Initialize Invert Mouse Y
    if (InvertMouseYCheckBox)
    {
        const bool bInvertY = HorrorGameSettings->GetInvertMouseY();
        InvertMouseYCheckBox->SetIsChecked(bInvertY);
        InvertMouseYCheckBox->OnCheckStateChanged.Clear();
        InvertMouseYCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsWidget::OnInvertMouseYChanged);

        UE_LOG(LogTemp, Log, TEXT("Invert Mouse Y initialized: %s"), bInvertY ? TEXT("True") : TEXT("False"));
    }

    // Initialize Invert Mouse X
    if (InvertMouseXCheckBox)
    {
        const bool bInvertX = HorrorGameSettings->GetInvertMouseX();
        InvertMouseXCheckBox->SetIsChecked(bInvertX);
        InvertMouseXCheckBox->OnCheckStateChanged.Clear();
        InvertMouseXCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsWidget::OnInvertMouseXChanged);

        UE_LOG(LogTemp, Log, TEXT("Invert Mouse X initialized: %s"), bInvertX ? TEXT("True") : TEXT("False"));
    }
}

void UControlsWidget::InitializeControllerSettings()
{
    if (!HorrorGameSettings)
        return;

    // Initialize Controller Sensitivity
    if (ControllerSensitivitySlider && ControllerSensitivityText)
    {
        const float ControllerSens = HorrorGameSettings->GetControllerSensitivity();

        // Configure slider properties
        ControllerSensitivitySlider->SetMinValue(ControlsWidgetPrivate::MinSensitivity);
        ControllerSensitivitySlider->SetMaxValue(ControlsWidgetPrivate::MaxSensitivity);
        ControllerSensitivitySlider->SetStepSize(ControlsWidgetPrivate::SensitivityStep);
        ControllerSensitivitySlider->SetValue(ControllerSens);

        // Update text display
        UpdateSensitivityText(ControllerSensitivityText, ControllerSens);

        // Bind event handler
        ControllerSensitivitySlider->OnValueChanged.Clear();
        ControllerSensitivitySlider->OnValueChanged.AddDynamic(this, &UControlsWidget::OnControllerSensitivityChanged);

        UE_LOG(LogTemp, Log, TEXT("Controller sensitivity initialized: %.2f"), ControllerSens);
    }

    // Initialize Controller Vibration
    if (ControllerVibrationCheckBox)
    {
        const bool bVibrationEnabled = HorrorGameSettings->GetControllerVibrationEnabled();
        ControllerVibrationCheckBox->SetIsChecked(bVibrationEnabled);
        ControllerVibrationCheckBox->OnCheckStateChanged.Clear();
        ControllerVibrationCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsWidget::OnControllerVibrationChanged);

        UE_LOG(LogTemp, Log, TEXT("Controller vibration initialized: %s"), bVibrationEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    }

    // Initialize Controller Dead Zone
    if (ControllerDeadZoneSlider && ControllerDeadZoneText)
    {
        const float DeadZone = HorrorGameSettings->GetControllerDeadZone();

        // Configure slider properties
        ControllerDeadZoneSlider->SetMinValue(ControlsWidgetPrivate::MinDeadZone);
        ControllerDeadZoneSlider->SetMaxValue(ControlsWidgetPrivate::MaxDeadZone);
        ControllerDeadZoneSlider->SetStepSize(ControlsWidgetPrivate::DeadZoneStep);
        ControllerDeadZoneSlider->SetValue(DeadZone);

        // Update text display
        ControllerDeadZoneText->SetText(FText::AsPercent(DeadZone));

        // Bind event handler
        ControllerDeadZoneSlider->OnValueChanged.Clear();
        ControllerDeadZoneSlider->OnValueChanged.AddDynamic(this, &UControlsWidget::OnControllerDeadZoneChanged);

        UE_LOG(LogTemp, Log, TEXT("Controller dead zone initialized: %.2f"), DeadZone);
    }
}

void UControlsWidget::InitializeAdvancedSettings()
{
    if (!HorrorGameSettings)
        return;

    // Initialize Mouse Smoothing
    if (MouseSmoothingCheckBox)
    {
        const bool bSmoothingEnabled = HorrorGameSettings->GetMouseSmoothingEnabled();
        MouseSmoothingCheckBox->SetIsChecked(bSmoothingEnabled);
        MouseSmoothingCheckBox->OnCheckStateChanged.Clear();
        MouseSmoothingCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsWidget::OnMouseSmoothingChanged);

        UE_LOG(LogTemp, Log, TEXT("Mouse smoothing initialized: %s"), bSmoothingEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    }

    // Initialize Mouse Acceleration
    if (MouseAccelerationCheckBox)
    {
        const bool bAccelerationEnabled = HorrorGameSettings->GetMouseAccelerationEnabled();
        MouseAccelerationCheckBox->SetIsChecked(bAccelerationEnabled);
        MouseAccelerationCheckBox->OnCheckStateChanged.Clear();
        MouseAccelerationCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsWidget::OnMouseAccelerationChanged);

        UE_LOG(LogTemp, Log, TEXT("Mouse acceleration initialized: %s"), bAccelerationEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    }

    // Initialize Raw Input
    if (RawInputCheckBox)
    {
        const bool bRawInputEnabled = HorrorGameSettings->GetRawInputEnabled();
        RawInputCheckBox->SetIsChecked(bRawInputEnabled);
        RawInputCheckBox->OnCheckStateChanged.Clear();
        RawInputCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsWidget::OnRawInputChanged);

        UE_LOG(LogTemp, Log, TEXT("Raw input initialized: %s"), bRawInputEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void UControlsWidget::InitializeKeyBindings()
{
    // Define default key bindings for horror game
    DefaultKeyBindings = {
        // Movement
        {FText::FromString("Move Forward"), FName("MoveForward"), EKeys::W, EKeys::Gamepad_LeftStick_Up},
        {FText::FromString("Move Backward"), FName("MoveBackward"), EKeys::S, EKeys::Gamepad_LeftStick_Down},
        {FText::FromString("Move Left"), FName("MoveLeft"), EKeys::A, EKeys::Gamepad_LeftStick_Left},
        {FText::FromString("Move Right"), FName("MoveRight"), EKeys::D, EKeys::Gamepad_LeftStick_Right},

        // Camera
        {FText::FromString("Look Up"), FName("LookUp"), EKeys::MouseY, EKeys::Gamepad_RightStick_Up},
        {FText::FromString("Look Right"), FName("LookRight"), EKeys::MouseX, EKeys::Gamepad_RightStick_Right},

        // Actions
        {FText::FromString("Jump"), FName("Jump"), EKeys::SpaceBar, EKeys::Gamepad_FaceButton_Bottom},
        {FText::FromString("Crouch"), FName("Crouch"), EKeys::LeftControl, EKeys::Gamepad_RightThumbstick},
        {FText::FromString("Run"), FName("Run"), EKeys::LeftShift, EKeys::Gamepad_LeftShoulder},
        {FText::FromString("Walk"), FName("Walk"), EKeys::LeftAlt, EKeys::Gamepad_LeftTrigger},

        // Interactions
        {FText::FromString("Interact"), FName("Interact"), EKeys::E, EKeys::Gamepad_FaceButton_Right},
        {FText::FromString("Use Item"), FName("UseItem"), EKeys::LeftMouseButton, EKeys::Gamepad_RightTrigger},
        {FText::FromString("Drop Item"), FName("DropItem"), EKeys::G, EKeys::Gamepad_FaceButton_Top},

        // Horror-specific
        {FText::FromString("Flashlight"), FName("Flashlight"), EKeys::F, EKeys::Gamepad_FaceButton_Left},
        {FText::FromString("Hide"), FName("Hide"), EKeys::C, EKeys::Gamepad_RightShoulder},
        {FText::FromString("Hold Breath"), FName("HoldBreath"), EKeys::LeftShift, EKeys::Gamepad_LeftTrigger},
        {FText::FromString("Lean Left"), FName("LeanLeft"), EKeys::Q, EKeys::Gamepad_DPad_Left},
        {FText::FromString("Lean Right"), FName("LeanRight"), EKeys::E, EKeys::Gamepad_DPad_Right},

        // UI
        {FText::FromString("Inventory"), FName("Inventory"), EKeys::Tab, EKeys::Gamepad_Special_Right},
        {FText::FromString("Pause Menu"), FName("PauseMenu"), EKeys::Escape, EKeys::Gamepad_Special_Left},
        {FText::FromString("Journal"), FName("Journal"), EKeys::J, EKeys::Gamepad_DPad_Up},
        {FText::FromString("Map"), FName("Map"), EKeys::M, EKeys::Gamepad_DPad_Down}
    };

    // Populate key binding widgets
    PopulateKeyBindings();

    // Initialize reset button
    if (ResetToDefaultsButton)
    {
        ResetToDefaultsButton->OnClicked.Clear();
        ResetToDefaultsButton->OnClicked.AddDynamic(this, &UControlsWidget::OnResetToDefaults);
    }

    UE_LOG(LogTemp, Log, TEXT("Key bindings initialized with %d actions"), DefaultKeyBindings.Num());
}

void UControlsWidget::PopulateKeyBindings()
{
    if (!KeyBindingsScrollBox || !KeyBindingWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PopulateKeyBindings: Missing required components"));
        return;
    }

    // Clear existing key binding widgets
    KeyBindingsScrollBox->ClearChildren();
    KeyBindingWidgets.Empty();

    // Create key binding widget for each action
    for (const FKeyBinding& Binding : DefaultKeyBindings)
    {
        UKeyBindingWidget* KeyBindingWidget = CreateWidget<UKeyBindingWidget>(this, KeyBindingWidgetClass);
        if (!KeyBindingWidget)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create KeyBindingWidget for action: %s"), *Binding.ActionTag.ToString());
            continue;
        }

        // Get current key binding from settings
        FKeyBinding CurrentBinding = Binding;
        if (HorrorGameSettings)
        {
            CurrentBinding.CurrentKey = HorrorGameSettings->GetKeyBinding(Binding.ActionTag, Binding.DefaultKey);
            CurrentBinding.CurrentGamepadKey = HorrorGameSettings->GetGamepadKeyBinding(Binding.ActionTag, Binding.DefaultGamepadKey);
        }

        // Setup the widget
        KeyBindingWidget->SetKeyBinding(CurrentBinding);
        KeyBindingWidget->OnKeyBindingChanged.BindLambda([this](FName ActionTag, FKey NewKey, bool bIsGamepadKey)
            {
                OnKeyBindingChanged(ActionTag, NewKey, bIsGamepadKey);
            });

        // Add to scroll box and track reference
        KeyBindingsScrollBox->AddChild(KeyBindingWidget);
        KeyBindingWidgets.Add(KeyBindingWidget);
    }

    UE_LOG(LogTemp, Log, TEXT("Created %d key binding widgets"), KeyBindingWidgets.Num());
}

void UControlsWidget::OnMouseSensitivityChanged(float Value)
{
    if (!HorrorGameSettings || !MouseSensitivityText)
        return;

    // Clamp value to valid range
    const float ClampedValue = FMath::Clamp(Value, ControlsWidgetPrivate::MinSensitivity, ControlsWidgetPrivate::MaxSensitivity);

    // Update settings
    HorrorGameSettings->SetMouseSensitivity(ClampedValue);
    HorrorGameSettings->SaveConfig();

    // Update text display
    UpdateSensitivityText(MouseSensitivityText, ClampedValue);

    // Apply to player controller if in game
    ApplyMouseSensitivityToPlayer(ClampedValue);

    UE_LOG(LogTemp, Log, TEXT("Mouse sensitivity changed to: %.2f"), ClampedValue);
}

void UControlsWidget::OnInvertMouseYChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
        return;

    HorrorGameSettings->SetInvertMouseY(bIsChecked);
    HorrorGameSettings->SaveConfig();

    // Apply to player controller if in game
    ApplyMouseInversionToPlayer();

    UE_LOG(LogTemp, Log, TEXT("Invert Mouse Y: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
}

void UControlsWidget::OnInvertMouseXChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
        return;

    HorrorGameSettings->SetInvertMouseX(bIsChecked);
    HorrorGameSettings->SaveConfig();

    // Apply to player controller if in game
    ApplyMouseInversionToPlayer();

    UE_LOG(LogTemp, Log, TEXT("Invert Mouse X: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
}

void UControlsWidget::OnControllerSensitivityChanged(float Value)
{
    if (!HorrorGameSettings || !ControllerSensitivityText)
        return;

    // Clamp value to valid range
    const float ClampedValue = FMath::Clamp(Value, ControlsWidgetPrivate::MinSensitivity, ControlsWidgetPrivate::MaxSensitivity);

    // Update settings
    HorrorGameSettings->SetControllerSensitivity(ClampedValue);
    HorrorGameSettings->SaveConfig();

    // Update text display
    UpdateSensitivityText(ControllerSensitivityText, ClampedValue);

    UE_LOG(LogTemp, Log, TEXT("Controller sensitivity changed to: %.2f"), ClampedValue);
}

void UControlsWidget::OnControllerVibrationChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
        return;

    HorrorGameSettings->SetControllerVibrationEnabled(bIsChecked);
    HorrorGameSettings->SaveConfig();

    UE_LOG(LogTemp, Log, TEXT("Controller vibration: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
}

void UControlsWidget::OnControllerDeadZoneChanged(float Value)
{
    if (!HorrorGameSettings || !ControllerDeadZoneText)
        return;

    // Clamp value to valid range
    const float ClampedValue = FMath::Clamp(Value, ControlsWidgetPrivate::MinDeadZone, ControlsWidgetPrivate::MaxDeadZone);

    // Update settings
    HorrorGameSettings->SetControllerDeadZone(ClampedValue);
    HorrorGameSettings->SaveConfig();

    // Update text display
    ControllerDeadZoneText->SetText(FText::AsPercent(ClampedValue));

    UE_LOG(LogTemp, Log, TEXT("Controller dead zone changed to: %.2f"), ClampedValue);
}

void UControlsWidget::OnMouseSmoothingChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
        return;

    HorrorGameSettings->SetMouseSmoothingEnabled(bIsChecked);
    HorrorGameSettings->SaveConfig();

    UE_LOG(LogTemp, Log, TEXT("Mouse smoothing: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
}

void UControlsWidget::OnMouseAccelerationChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
        return;

    HorrorGameSettings->SetMouseAccelerationEnabled(bIsChecked);
    HorrorGameSettings->SaveConfig();

    UE_LOG(LogTemp, Log, TEXT("Mouse acceleration: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
}

void UControlsWidget::OnRawInputChanged(bool bIsChecked)
{
    if (!HorrorGameSettings)
        return;

    HorrorGameSettings->SetRawInputEnabled(bIsChecked);
    HorrorGameSettings->SaveConfig();

    UE_LOG(LogTemp, Log, TEXT("Raw input: %s"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
}

void UControlsWidget::OnKeyBindingChanged(FName ActionTag, FKey NewKey, bool bIsGamepadKey)
{
    if (!HorrorGameSettings)
        return;

    // Check for key conflicts
    if (HasKeyConflict(NewKey, ActionTag, bIsGamepadKey))
    {
        UE_LOG(LogTemp, Warning, TEXT("Key conflict detected for action %s with key %s"), *ActionTag.ToString(), *NewKey.ToString());
        // Could show a confirmation dialog here
    }

    // Update the key binding
    if (bIsGamepadKey)
    {
        HorrorGameSettings->SetGamepadKeyBinding(ActionTag, NewKey);
    }
    else
    {
        HorrorGameSettings->SetKeyBinding(ActionTag, NewKey);
    }

    HorrorGameSettings->SaveConfig();

    // Update input settings
    UpdateInputSettings(ActionTag, NewKey, bIsGamepadKey);

    UE_LOG(LogTemp, Log, TEXT("Key binding changed: %s -> %s (%s)"),
        *ActionTag.ToString(),
        *NewKey.ToString(),
        bIsGamepadKey ? TEXT("Gamepad") : TEXT("Keyboard/Mouse"));
}

void UControlsWidget::OnResetToDefaults()
{
    if (!HorrorGameSettings)
        return;

    UE_LOG(LogTemp, Log, TEXT("Resetting controls to defaults..."));

    // Reset all control settings to defaults
    ResetToDefaults();
}

void UControlsWidget::RefreshValues()
{
    if (!HorrorGameSettings)
    {
        HorrorGameSettings = UHorrorGameSettings::Get();
        if (!HorrorGameSettings)
        {
            UE_LOG(LogTemp, Error, TEXT("RefreshValues: Failed to get HorrorGameSettings"));
            return;
        }
    }

    // Refresh all settings
    InitializeMouseSettings();
    InitializeControllerSettings();
    InitializeAdvancedSettings();
    PopulateKeyBindings();

    UE_LOG(LogTemp, Log, TEXT("Controls settings refreshed"));
}

void UControlsWidget::ResetToDefaults()
{
    if (!HorrorGameSettings)
    {
        HorrorGameSettings = UHorrorGameSettings::Get();
        if (!HorrorGameSettings)
        {
            UE_LOG(LogTemp, Error, TEXT("ResetToDefaults: Failed to get HorrorGameSettings"));
            return;
        }
    }

    // Reset mouse settings to defaults
    HorrorGameSettings->SetMouseSensitivity(ControlsWidgetPrivate::DefaultMouseSensitivity);
    HorrorGameSettings->SetInvertMouseY(false);
    HorrorGameSettings->SetInvertMouseX(false);
    HorrorGameSettings->SetMouseSmoothingEnabled(false);
    HorrorGameSettings->SetMouseAccelerationEnabled(false);
    HorrorGameSettings->SetRawInputEnabled(true);

    // Reset controller settings to defaults
    HorrorGameSettings->SetControllerSensitivity(ControlsWidgetPrivate::DefaultControllerSensitivity);
    HorrorGameSettings->SetControllerVibrationEnabled(true);
    HorrorGameSettings->SetControllerDeadZone(ControlsWidgetPrivate::DefaultControllerDeadZone);

    // Reset key bindings to defaults
    ResetKeyBindingsToDefaults();

    // Save all changes
    HorrorGameSettings->SaveConfig();

    // Refresh UI to show new values
    RefreshValues();

    UE_LOG(LogTemp, Log, TEXT("Controls settings reset to defaults"));
}

void UControlsWidget::ResetKeyBindingsToDefaults()
{
    if (!HorrorGameSettings)
        return;

    // Reset each key binding to its default value
    for (const FKeyBinding& Binding : DefaultKeyBindings)
    {
        HorrorGameSettings->SetKeyBinding(Binding.ActionTag, Binding.DefaultKey);
        HorrorGameSettings->SetGamepadKeyBinding(Binding.ActionTag, Binding.DefaultGamepadKey);

        // Update input settings
        UpdateInputSettings(Binding.ActionTag, Binding.DefaultKey, false);
        UpdateInputSettings(Binding.ActionTag, Binding.DefaultGamepadKey, true);
    }

    UE_LOG(LogTemp, Log, TEXT("Key bindings reset to defaults"));
}

bool UControlsWidget::ValidateControlsComponents()
{
    bool bAllValid = true;
    TArray<FString> MissingComponents;

    // Check core mouse components
    if (!MouseSensitivitySlider)
    {
        MissingComponents.Add(TEXT("MouseSensitivitySlider"));
        bAllValid = false;
    }

    if (!MouseSensitivityText)
    {
        MissingComponents.Add(TEXT("MouseSensitivityText"));
        bAllValid = false;
    }

    // Check key bindings components
    if (!KeyBindingsScrollBox)
    {
        MissingComponents.Add(TEXT("KeyBindingsScrollBox"));
        bAllValid = false;
    }

    if (!KeyBindingWidgetClass)
    {
        MissingComponents.Add(TEXT("KeyBindingWidgetClass"));
        bAllValid = false;
    }

    // Log missing optional components as warnings
    const TArray<TPair<UWidget*, FString>> OptionalWidgets = {
        {InvertMouseYCheckBox, TEXT("InvertMouseYCheckBox")},
        {InvertMouseXCheckBox, TEXT("InvertMouseXCheckBox")},
        {ControllerSensitivitySlider, TEXT("ControllerSensitivitySlider")},
        {ControllerSensitivityText, TEXT("ControllerSensitivityText")},
        {ControllerVibrationCheckBox, TEXT("ControllerVibrationCheckBox")},
        {MouseSmoothingCheckBox, TEXT("MouseSmoothingCheckBox")},
        {MouseAccelerationCheckBox, TEXT("MouseAccelerationCheckBox")},
        {RawInputCheckBox, TEXT("RawInputCheckBox")},
        {ResetToDefaultsButton, TEXT("ResetToDefaultsButton")}
    };

    for (const auto& [Widget, Name] : OptionalWidgets)
    {
        if (!Widget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Optional widget missing: %s"), *Name);
        }
    }

    // Check HorrorGameSettings
    if (!HorrorGameSettings)
    {
        MissingComponents.Add(TEXT("HorrorGameSettings"));
        bAllValid = false;
    }

    // Log validation results
    if (bAllValid)
    {
        UE_LOG(LogTemp, Log, TEXT("ControlsWidget component validation passed"));
    }
    else
    {
        FString MissingList = FString::Join(MissingComponents, TEXT(", "));
        UE_LOG(LogTemp, Error, TEXT("ControlsWidget validation failed. Missing components: %s"), *MissingList);
    }

    return bAllValid;
}

void UControlsWidget::UpdateSensitivityText(UTextBlock* TextWidget, float Value)
{
    if (!TextWidget)
        return;

    // Format sensitivity value with 2 decimal places
    FNumberFormattingOptions Options;
    Options.MaximumFractionalDigits = 2;
    Options.MinimumFractionalDigits = 1;

    TextWidget->SetText(FText::AsNumber(Value, &Options));
}

void UControlsWidget::ApplyMouseSensitivityToPlayer(float Sensitivity)
{
    // Apply sensitivity to current player controller if in game
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (ASettingsController* SettingPC = Cast<ASettingsController>(PC))
            {
                float CurrentYawScale = SettingPC->getInputYawnScale();
                float CurrentPitchScale = SettingPC->getInputPitchScale();
                CurrentYawScale = Sensitivity;
                CurrentPitchScale = Sensitivity;
            }
        }
    }
}

void UControlsWidget::ApplyMouseInversionToPlayer()
{
    if (!HorrorGameSettings)
        return;

    // Apply mouse inversion to current player controller if in game
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (ASettingsController* SettingPC = Cast<ASettingsController>(PC))
            {
                const bool bInvertX = HorrorGameSettings->GetInvertMouseX();
                const bool bInvertY = HorrorGameSettings->GetInvertMouseY();

                float CurrentYawScale = SettingPC->getInputYawnScale();
                float CurrentPitchScale = SettingPC->getInputPitchScale();

                SettingPC->setInputYawScale(bInvertX ? -FMath::Abs(CurrentYawScale) : FMath::Abs(CurrentYawScale));
                SettingPC->setInputPitchScale(bInvertY ? -FMath::Abs(CurrentPitchScale) : FMath::Abs(CurrentPitchScale));
            }
        }
    }
}

bool UControlsWidget::HasKeyConflict(FKey NewKey, FName ExcludeAction, bool bIsGamepadKey)
{
    if (!HorrorGameSettings)
        return false;

    // Check for conflicts with other actions
    for (const FKeyBinding& Binding : DefaultKeyBindings)
    {
        if (Binding.ActionTag == ExcludeAction)
            continue;

        FKey CurrentKey = bIsGamepadKey ?
            HorrorGameSettings->GetGamepadKeyBinding(Binding.ActionTag, Binding.DefaultGamepadKey) :
            HorrorGameSettings->GetKeyBinding(Binding.ActionTag, Binding.DefaultKey);

        if (CurrentKey == NewKey)
        {
            return true;
        }
    }

    return false;
}

void UControlsWidget::UpdateInputSettings(FName ActionName, FKey Key, bool bIsGamepadKey)
{
    // Update the Unreal Engine input settings
    if (UInputSettings* InputSettings = UInputSettings::GetInputSettings())
    {
        // This would typically update action mappings
        // Implementation depends on how your input system is set up
        InputSettings->SaveKeyMappings();
        UE_LOG(LogTemp, Log, TEXT("Updated input settings for action: %s"), *ActionName.ToString());
    }
}

void UControlsWidget::ShowKeyConflictDialog(FName ConflictingAction, FKey Key)
{
    // Implementation for showing conflict resolution dialog
    // This could create a popup widget asking user how to resolve the conflict
    UE_LOG(LogTemp, Warning, TEXT("Key conflict: %s is already bound to action %s"), *Key.ToString(), *ConflictingAction.ToString());

    // For now, just log the conflict. In a full implementation, you would show a dialog
    // asking the user if they want to:
    // 1. Override the existing binding
    // 2. Cancel the new binding
    // 3. Assign a different key
}

void UControlsWidget::SaveControlsSettings()
{
    if (!HorrorGameSettings)
        return;

    // Save all control settings
    HorrorGameSettings->SaveConfig();

    // Apply settings to current game session
    ApplySettingsToGame();

    UE_LOG(LogTemp, Log, TEXT("Controls settings saved"));
}

void UControlsWidget::LoadControlsSettings()
{
    if (!HorrorGameSettings)
    {
        HorrorGameSettings = UHorrorGameSettings::Get();
        if (!HorrorGameSettings)
        {
            UE_LOG(LogTemp, Error, TEXT("LoadControlsSettings: Failed to get HorrorGameSettings"));
            return;
        }
    }

    // Load settings and refresh UI
    RefreshValues();

    UE_LOG(LogTemp, Log, TEXT("Controls settings loaded"));
}

void UControlsWidget::ApplySettingsToGame()
{
    if (!HorrorGameSettings)
        return;

    // Apply mouse settings
    ApplyMouseSensitivityToPlayer(HorrorGameSettings->GetMouseSensitivity());
    ApplyMouseInversionToPlayer();

    // Apply controller settings (if controller is connected)
    ApplyControllerSettings();

    UE_LOG(LogTemp, Log, TEXT("Controls settings applied to game"));
}

void UControlsWidget::ApplyControllerSettings()
{
    if (!HorrorGameSettings)
        return;

    // Apply controller settings to game
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            // Apply controller sensitivity, dead zone, etc.
            // This would be game-specific implementation
            UE_LOG(LogTemp, Log, TEXT("Applied controller settings to player controller"));
        }
    }
}

FString UControlsWidget::GetActionDisplayName(FName ActionName)
{
    // Convert action name to display-friendly string
    FString ActionString = ActionName.ToString();

    // Add spaces before capital letters for better readability
    FString DisplayName;
    for (int32 i = 0; i < ActionString.Len(); ++i)
    {
        if (i > 0 && FChar::IsUpper(ActionString[i]) && FChar::IsLower(ActionString[i - 1]))
        {
            DisplayName += TEXT(" ");
        }
        DisplayName += ActionString[i];
    }

    return DisplayName;
}

bool UControlsWidget::IsKeyReserved(FKey Key)
{
    // Define reserved keys that shouldn't be reassigned
    static const TArray<FKey> ReservedKeys = {
        EKeys::Escape,      // Usually reserved for menu
        EKeys::F4,          // Alt+F4 system shortcut
        EKeys::Tab,         // Often used for UI navigation
        EKeys::Enter,       // Enter/Return for UI
        EKeys::BackSpace    // Backspace for UI
    };

    return ReservedKeys.Contains(Key);
}

void UControlsWidget::ExportKeyBindings()
{
    if (!HorrorGameSettings)
        return;

    // Export current key bindings to a file or clipboard
    FString ExportData;
    ExportData += TEXT("# Horror Game Key Bindings Export\n");
    ExportData += FString::Printf(TEXT("# Exported on %s\n"), *FDateTime::Now().ToString());
    ExportData += TEXT("\n");

    for (const FKeyBinding& Binding : DefaultKeyBindings)
    {
        FKey CurrentKey = HorrorGameSettings->GetKeyBinding(Binding.ActionTag, Binding.DefaultKey);
        FKey CurrentGamepadKey = HorrorGameSettings->GetGamepadKeyBinding(Binding.ActionTag, Binding.DefaultGamepadKey);

        ExportData += FString::Printf(TEXT("%s=%s,%s\n"),
            *Binding.ActionTag.ToString(),
            *CurrentKey.ToString(),
            *CurrentGamepadKey.ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("Key bindings exported:\n%s"), *ExportData);

    // In a full implementation, you would save this to a file or copy to clipboard
}

void UControlsWidget::ImportKeyBindings(const FString& ImportData)
{
    if (!HorrorGameSettings || ImportData.IsEmpty())
        return;

    // Parse the import data and apply key bindings
    TArray<FString> Lines;
    ImportData.ParseIntoArrayLines(Lines);

    int32 ImportedCount = 0;
    for (const FString& Line : Lines)
    {
        // Skip comments and empty lines
        if (Line.IsEmpty() || Line.StartsWith(TEXT("#")))
            continue;

        FString ActionName, Keys;
        if (Line.Split(TEXT("="), &ActionName, &Keys))
        {
            FString KeyboardKey, GamepadKey;
            if (Keys.Split(TEXT(","), &KeyboardKey, &GamepadKey))
            {
                FName ActionTag(*ActionName);

                // Apply the imported key bindings
                if (!KeyboardKey.IsEmpty())
                {
                    FKey Key(*KeyboardKey);
                    if (Key.IsValid())
                    {
                        HorrorGameSettings->SetKeyBinding(ActionTag, Key);
                        ImportedCount++;
                    }
                }

                if (!GamepadKey.IsEmpty())
                {
                    FKey Key(*GamepadKey);
                    if (Key.IsValid())
                    {
                        HorrorGameSettings->SetGamepadKeyBinding(ActionTag, Key);
                        ImportedCount++;
                    }
                }
            }
        }
    }

    if (ImportedCount > 0)
    {
        HorrorGameSettings->SaveConfig();
        RefreshValues();
        UE_LOG(LogTemp, Log, TEXT("Imported %d key bindings"), ImportedCount);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid key bindings found in import data"));
    }
}

void UControlsWidget::TestKeyBinding(FName ActionName)
{
    if (!HorrorGameSettings)
        return;

    // Test if the key binding works by simulating input
    FKey TestKey = HorrorGameSettings->GetKeyBinding(ActionName, EKeys::Invalid);
    if (TestKey.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Testing key binding: %s -> %s"), *ActionName.ToString(), *TestKey.ToString());

        // In a full implementation, you might briefly highlight the key or show feedback
        // For now, just log the test
    }
}

void UControlsWidget::OnKeyBindingStarted(FName ActionName)
{
    // Called when user starts changing a key binding
    CurrentlyRebindingAction = ActionName;
    UE_LOG(LogTemp, Log, TEXT("Started rebinding action: %s"), *ActionName.ToString());
}

void UControlsWidget::OnKeyBindingCancelled(FName ActionName)
{
    // Called when user cancels key binding change
    CurrentlyRebindingAction = NAME_None;
    UE_LOG(LogTemp, Log, TEXT("Cancelled rebinding action: %s"), *ActionName.ToString());
}

void UControlsWidget::OnKeyBindingCompleted(FName ActionName, FKey NewKey)
{
    // Called when user successfully changes a key binding
    CurrentlyRebindingAction = NAME_None;
    UE_LOG(LogTemp, Log, TEXT("Completed rebinding action: %s to %s"), *ActionName.ToString(), *NewKey.ToString());
}

void UControlsWidget::LogCurrentControlsSettings()
{
    if (!HorrorGameSettings)
        return;

    UE_LOG(LogTemp, Log, TEXT("=== Current Controls Settings ==="));

    // Mouse settings
    UE_LOG(LogTemp, Log, TEXT("Mouse Sensitivity: %.2f"), HorrorGameSettings->GetMouseSensitivity());
    UE_LOG(LogTemp, Log, TEXT("Invert Mouse X: %s"), HorrorGameSettings->GetInvertMouseX() ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Invert Mouse Y: %s"), HorrorGameSettings->GetInvertMouseY() ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Mouse Smoothing: %s"), HorrorGameSettings->GetMouseSmoothingEnabled() ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Log, TEXT("Mouse Acceleration: %s"), HorrorGameSettings->GetMouseAccelerationEnabled() ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Log, TEXT("Raw Input: %s"), HorrorGameSettings->GetRawInputEnabled() ? TEXT("Enabled") : TEXT("Disabled"));

    // Controller settings
    UE_LOG(LogTemp, Log, TEXT("Controller Sensitivity: %.2f"), HorrorGameSettings->GetControllerSensitivity());
    UE_LOG(LogTemp, Log, TEXT("Controller Vibration: %s"), HorrorGameSettings->GetControllerVibrationEnabled() ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Log, TEXT("Controller Dead Zone: %.2f"), HorrorGameSettings->GetControllerDeadZone());

    // Key bindings
    UE_LOG(LogTemp, Log, TEXT("=== Key Bindings ==="));
    for (const FKeyBinding& Binding : DefaultKeyBindings)
    {
        FKey CurrentKey = HorrorGameSettings->GetKeyBinding(Binding.ActionTag, Binding.DefaultKey);
        FKey CurrentGamepadKey = HorrorGameSettings->GetGamepadKeyBinding(Binding.ActionTag, Binding.DefaultGamepadKey);
        UE_LOG(LogTemp, Log, TEXT("%s: %s / %s"),
            *Binding.ActionName.ToString(),
            *CurrentKey.ToString(),
            *CurrentGamepadKey.ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Controls Settings ==="));
}

bool UControlsWidget::AreControlsAtDefaults()
{
    if (!HorrorGameSettings)
        return false;

    // Check if all settings are at their default values
    bool bAtDefaults = true;

    // Check mouse settings
    bAtDefaults &= FMath::IsNearlyEqual(HorrorGameSettings->GetMouseSensitivity(), ControlsWidgetPrivate::DefaultMouseSensitivity, 0.01f);
    bAtDefaults &= !HorrorGameSettings->GetInvertMouseX();
    bAtDefaults &= !HorrorGameSettings->GetInvertMouseY();
    bAtDefaults &= !HorrorGameSettings->GetMouseSmoothingEnabled();
    bAtDefaults &= !HorrorGameSettings->GetMouseAccelerationEnabled();
    bAtDefaults &= HorrorGameSettings->GetRawInputEnabled();

    // Check controller settings
    bAtDefaults &= FMath::IsNearlyEqual(HorrorGameSettings->GetControllerSensitivity(), ControlsWidgetPrivate::DefaultControllerSensitivity, 0.01f);
    bAtDefaults &= HorrorGameSettings->GetControllerVibrationEnabled();
    bAtDefaults &= FMath::IsNearlyEqual(HorrorGameSettings->GetControllerDeadZone(), ControlsWidgetPrivate::DefaultControllerDeadZone, 0.01f);

    // Check key bindings
    for (const FKeyBinding& Binding : DefaultKeyBindings)
    {
        FKey CurrentKey = HorrorGameSettings->GetKeyBinding(Binding.ActionTag, Binding.DefaultKey);
        FKey CurrentGamepadKey = HorrorGameSettings->GetGamepadKeyBinding(Binding.ActionTag, Binding.DefaultGamepadKey);

        bAtDefaults &= (CurrentKey == Binding.DefaultKey);
        bAtDefaults &= (CurrentGamepadKey == Binding.DefaultGamepadKey);

        if (!bAtDefaults)
            break;
    }

    return bAtDefaults;
}

void UControlsWidget::SetControlsPreset(EControlsPreset Preset)
{
    if (!HorrorGameSettings)
        return;

    UE_LOG(LogTemp, Log, TEXT("Applying controls preset: %d"), (int32)Preset);

    switch (Preset)
    {
    case EControlsPreset::Default:
        ResetToDefaults();
        break;

    case EControlsPreset::HighSensitivity:
        HorrorGameSettings->SetMouseSensitivity(2.5f);
        HorrorGameSettings->SetControllerSensitivity(2.0f);
        break;

    case EControlsPreset::LowSensitivity:
        HorrorGameSettings->SetMouseSensitivity(0.5f);
        HorrorGameSettings->SetControllerSensitivity(0.7f);
        break;

    case EControlsPreset::Accessibility:
        // Preset for accessibility - longer hold times, different bindings, etc.
        HorrorGameSettings->SetMouseSensitivity(0.8f);
        HorrorGameSettings->SetControllerDeadZone(0.35f);
        HorrorGameSettings->SetControllerVibrationEnabled(false);
        break;
    }

    HorrorGameSettings->SaveConfig();
    RefreshValues();

    UE_LOG(LogTemp, Log, TEXT("Controls preset applied"));
}