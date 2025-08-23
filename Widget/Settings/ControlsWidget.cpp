#include "HorrorGame/Widget/Settings/ControlsWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "HorrorGame/Widget/Settings/KeyBindingWidget.h"

void UControlsWidget::NativeConstruct()
{
    HorrorGameSettings = UHorrorGameSettings::Get();

    InitializeMouseSettings();
    InitializeControllerSettings();
    InitializeKeyBindings();
}

UWidget* UControlsWidget::NativeGetDesiredFocusTarget() const
{
    return MouseSensitivitySlider;
}

void UControlsWidget::InitializeMouseSettings()
{
    // Mouse Sensitivity
    MouseSensitivitySlider->SetValue(HorrorGameSettings->GetMouseSensitivity());
    MouseSensitivityText->SetText(FText::AsNumber(HorrorGameSettings->GetMouseSensitivity(), &FNumberFormattingOptions::DefaultNoGrouping()));
    MouseSensitivitySlider->OnValueChanged.Clear();
    MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UControlsWidget::OnMouseSensitivityChanged);

    // Invert Mouse
    InvertMouseYCheckBox->SetIsChecked(HorrorGameSettings->GetInvertMouseY());
    InvertMouseYCheckBox->OnCheckStateChanged.Clear();
    InvertMouseYCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsWidget::OnInvertMouseYChanged);

    InvertMouseXCheckBox->SetIsChecked(HorrorGameSettings->GetInvertMouseX());
    InvertMouseXCheckBox->OnCheckStateChanged.Clear();
    InvertMouseXCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsWidget::OnInvertMouseXChanged);
}

void UControlsWidget::InitializeControllerSettings()
{
    // Controller Sensitivity
    ControllerSensitivitySlider->SetValue(HorrorGameSettings->GetControllerSensitivity());
    ControllerSensitivityText->SetText(FText::AsNumber(HorrorGameSettings->GetControllerSensitivity(), &FNumberFormattingOptions::DefaultNoGrouping()));
    ControllerSensitivitySlider->OnValueChanged.Clear();
    ControllerSensitivitySlider->OnValueChanged.AddDynamic(this, &UControlsWidget::OnControllerSensitivityChanged);

    // Controller Vibration
    ControllerVibrationCheckBox->SetIsChecked(HorrorGameSettings->GetControllerVibrationEnabled());
    ControllerVibrationCheckBox->OnCheckStateChanged.Clear();
    ControllerVibrationCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsWidget::OnControllerVibrationChanged);
}

void UControlsWidget::InitializeKeyBindings()
{
    // Setup default key bindings
    DefaultKeyBindings = {
        {FText::FromString("Move Forward"), FName("MoveForward"), EKeys::W},
        {FText::FromString("Move Backward"), FName("MoveBackward"), EKeys::S},
        {FText::FromString("Move Left"), FName("MoveLeft"), EKeys::A},
        {FText::FromString("Move Right"), FName("MoveRight"), EKeys::D},
        {FText::FromString("Look Up"), FName("LookUp"), EKeys::MouseY},
        {FText::FromString("Look Right"), FName("LookRight"), EKeys::MouseX},
        {FText::FromString("Jump"), FName("Jump"), EKeys::SpaceBar},
        {FText::FromString("Crouch"), FName("Crouch"), EKeys::LeftControl},
        {FText::FromString("Run"), FName("Run"), EKeys::LeftShift},
        {FText::FromString("Interact"), FName("Interact"), EKeys::E},
        {FText::FromString("Flashlight"), FName("Flashlight"), EKeys::F},
        {FText::FromString("Inventory"), FName("Inventory"), EKeys::Tab},
        {FText::FromString("Pause Menu"), FName("PauseMenu"), EKeys::Escape},
        {FText::FromString("Use Item"), FName("UseItem"), EKeys::LeftMouseButton},
        {FText::FromString("Drop Item"), FName("DropItem"), EKeys::G},
        {FText::FromString("Hide"), FName("Hide"), EKeys::LeftControl}
    };

    PopulateKeyBindings();

    // Reset button
    ResetToDefaultsButton->OnClicked.Clear();
    ResetToDefaultsButton->OnClicked.AddDynamic(this, &UControlsWidget::OnResetToDefaults);
}

void UControlsWidget::PopulateKeyBindings()
{
    KeyBindingsScrollBox->ClearChildren();

    for (const FKeyBinding& Binding : DefaultKeyBindings)
    {
        if (KeyBindingWidgetClass)
        {
            UKeyBindingWidget* KeyBindingWidget = CreateWidget<UKeyBindingWidget>(this, KeyBindingWidgetClass);
            if (KeyBindingWidget)
            {
                FKeyBinding CurrentBinding = Binding;
                CurrentBinding.CurrentKey = HorrorGameSettings->GetKeyBinding(Binding.ActionTag, Binding.DefaultKey);

                KeyBindingWidget->SetKeyBinding(CurrentBinding);
                KeyBindingWidget->OnKeyBindingChanged.BindLambda([this](FName ActionTag, FKey NewKey)
                    {
                        HorrorGameSettings->SetKeyBinding(ActionTag, NewKey);
                        HorrorGameSettings->SaveConfig();
                    });

                KeyBindingsScrollBox->AddChild(KeyBindingWidget);
            }
        }
    }
}

void UControlsWidget::OnMouseSensitivityChanged(float Value)
{
    HorrorGameSettings->SetMouseSensitivity(Value);
    HorrorGameSettings->SaveConfig();
    MouseSensitivityText->SetText(FText::AsNumber(Value, &FNumberFormattingOptions::DefaultNoGrouping()));
}

void UControlsWidget::OnInvertMouseYChanged(bool bIsChecked)
{
    HorrorGameSettings->SetInvertMouseY(bIsChecked);
    HorrorGameSettings->SaveConfig();
}

void UControlsWidget::OnInvertMouseXChanged(bool bIsChecked)
{
    HorrorGameSettings->SetInvertMouseX(bIsChecked);
    HorrorGameSettings->SaveConfig();
}

void UControlsWidget::OnControllerSensitivityChanged(float Value)
{
    HorrorGameSettings->SetControllerSensitivity(Value);
    HorrorGameSettings->SaveConfig();
    ControllerSensitivityText->SetText(FText::AsNumber(Value, &FNumberFormattingOptions::DefaultNoGrouping()));
}

void UControlsWidget::OnControllerVibrationChanged(bool bIsChecked)
{
    HorrorGameSettings->SetControllerVibrationEnabled(bIsChecked);
    HorrorGameSettings->SaveConfig();
}

void UControlsWidget::OnResetToDefaults()
{
    HorrorGameSettings->ResetKeyBindingsToDefault();
    HorrorGameSettings->SaveConfig();
    PopulateKeyBindings();
}