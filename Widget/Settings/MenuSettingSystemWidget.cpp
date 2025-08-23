#include "HorrorGame/Widget/Settings/MenuSettingSystemWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "GameplayWidget.h"
#include "GraphicsWidget.h"
#include "AudioWidget.h"
#include "ControlsWidget.h"
#include "AccessibilityWidget.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "GameFramework/GameUserSettings.h"

void UMenuSettingSystemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind tab buttons
    if (GameplayTabButton)
        GameplayTabButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnGameplayTabClicked);

    if (GraphicsTabButton)
        GraphicsTabButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnGraphicsTabClicked);

    if (AudioTabButton)
        AudioTabButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnAudioTabClicked);

    if (ControlsTabButton)
        ControlsTabButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnControlsTabClicked);

    if (AccessibilityTabButton)
        AccessibilityTabButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnAccessibilityTabClicked);

    // Bind action buttons
    if (ApplyButton)
        ApplyButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnApplyClicked);

    if (ResetButton)
        ResetButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnResetClicked);

    if (BackButton)
        BackButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnBackClicked);

    // Set default tab
    UpdateTabSelection(0);
}

UWidget* UMenuSettingSystemWidget::NativeGetDesiredFocusTarget() const
{
    return GameplayTabButton;
}

void UMenuSettingSystemWidget::OnGameplayTabClicked()
{
    UpdateTabSelection(0);
}

void UMenuSettingSystemWidget::OnGraphicsTabClicked()
{
    UpdateTabSelection(1);
}

void UMenuSettingSystemWidget::OnAudioTabClicked()
{
    UpdateTabSelection(2);
}

void UMenuSettingSystemWidget::OnControlsTabClicked()
{
    UpdateTabSelection(3);
}

void UMenuSettingSystemWidget::OnAccessibilityTabClicked()
{
    UpdateTabSelection(4);
}

void UMenuSettingSystemWidget::UpdateTabSelection(int32 SelectedTab)
{
    CurrentTab = SelectedTab;

    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(SelectedTab);
    }

    // Update tab button appearances
    SetTabButtonSelected(GameplayTabButton, SelectedTab == 0);
    SetTabButtonSelected(GraphicsTabButton, SelectedTab == 1);
    SetTabButtonSelected(AudioTabButton, SelectedTab == 2);
    SetTabButtonSelected(ControlsTabButton, SelectedTab == 3);
    SetTabButtonSelected(AccessibilityTabButton, SelectedTab == 4);
}

void UMenuSettingSystemWidget::SetTabButtonSelected(UButton* Button, bool bSelected)
{
    if (Button)
    {
        // TODO: Update button style based on selection state
        // You can implement this with different button styles in Blueprint
    }
}

void UMenuSettingSystemWidget::OnApplyClicked()
{
    // Apply all pending settings changes
    if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
    {
        GameUserSettings->ApplySettings(true);
    }

    if (UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get())
    {
        GameSettings->SaveConfig();
    }

    // Show confirmation message
    // TODO: Show "Settings Applied" notification
}

void UMenuSettingSystemWidget::OnResetClicked()
{
    // Show confirmation dialog
    // TODO: Implement confirmation dialog
    // For now, reset immediately

    if (UHorrorGameSettings* GameSettings = UHorrorGameSettings::Get())
    {
        GameSettings->ResetAllToDefaults();
    }

    if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
    {
        GameUserSettings->SetToDefaults();
        GameUserSettings->ApplySettings(true);
    }

    // Refresh all widgets to show new values
    // This would require rebuilding the widgets or updating their values
}

void UMenuSettingSystemWidget::OnBackClicked()
{
    // Check if there are unsaved changes
    // TODO: Implement unsaved changes detection and confirmation dialog

    // For now, just close the settings menu
    RemoveFromParent();
}