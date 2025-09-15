#include "SettingsInputHandler.h"
#include "HorrorGame/Widget/Settings/MainSettings/MenuSettingSystemWidget.h"
#include "HorrorGame/Object/Managers/SettingsTabManager.h"
#include "HorrorGame/Object/Managers/SettingsStateManager.h"

USettingsInputHandler::USettingsInputHandler()
    : ParentWidget(nullptr)
    , TabManager(nullptr)
    , StateManager(nullptr)
    , bKeyboardShortcutsEnabled(true)
{
}

void USettingsInputHandler::Initialize(UMenuSettingSystemWidget* InParentWidget)
{
    if (!InParentWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsInputHandler::Initialize - ParentWidget is null!"));
        return;
    }

    ParentWidget = InParentWidget;

    // Setup default shortcuts
    SetupKeyboardShortcuts();

    UE_LOG(LogTemp, Log, TEXT("SettingsInputHandler initialized"));
}

FReply USettingsInputHandler::HandleKeyDown(const FKeyEvent& InKeyEvent)
{
    if (!bKeyboardShortcutsEnabled)
    {
        return FReply::Unhandled();
    }

    FKey Key = InKeyEvent.GetKey();

    // Handle different types of input
    if (HandleActionShortcuts(InKeyEvent))
    {
        return FReply::Handled();
    }

    if (HandleTabNavigation(InKeyEvent))
    {
        return FReply::Handled();
    }

    if (HandleNumberShortcuts(InKeyEvent))
    {
        return FReply::Handled();
    }

    if (HandleSpecialKeys(InKeyEvent))
    {
        return FReply::Handled();
    }

    return FReply::Unhandled();
}

bool USettingsInputHandler::HandleControllerInput(const FKey& Key)
{
    // Handle gamepad input
    if (Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Gamepad_LeftStick_Left)
    {
        if (TabManager.IsValid())
        {
            TabManager->PreviousTab();
            return true;
        }
    }
    else if (Key == EKeys::Gamepad_DPad_Right || Key == EKeys::Gamepad_LeftStick_Right)
    {
        if (TabManager.IsValid())
        {
            TabManager->NextTab();
            return true;
        }
    }
    else if (Key == EKeys::Gamepad_FaceButton_Bottom) // A/X button
    {
        if (StateManager.IsValid() && StateManager->HasPendingChanges())
        {
            StateManager->ApplySettings();
            return true;
        }
    }
    else if (Key == EKeys::Gamepad_FaceButton_Right) // B/Circle button
    {
        if (StateManager.IsValid())
        {
            StateManager->CancelChanges();
            return true;
        }
    }

    return false;
}

void USettingsInputHandler::SetupKeyboardShortcuts()
{
    InitializeDefaultShortcuts();
    UE_LOG(LogTemp, Log, TEXT("SettingsInputHandler: Keyboard shortcuts setup complete"));
}

void USettingsInputHandler::EnableKeyboardShortcuts(bool bEnabled)
{
    bKeyboardShortcutsEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("SettingsInputHandler: Keyboard shortcuts %s"),
        bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void USettingsInputHandler::SetTabManager(USettingsTabManager* InTabManager)
{
    TabManager = InTabManager;
    UE_LOG(LogTemp, Log, TEXT("SettingsInputHandler: TabManager reference set"));
}

void USettingsInputHandler::SetStateManager(USettingsStateManager* InStateManager)
{
    StateManager = InStateManager;
    UE_LOG(LogTemp, Log, TEXT("SettingsInputHandler: StateManager reference set"));
}

// === INPUT PROCESSING ===

bool USettingsInputHandler::HandleActionShortcuts(const FKeyEvent& InKeyEvent)
{
    FKey Key = InKeyEvent.GetKey();
    bool bIsControlDown = InKeyEvent.IsControlDown();

    if (bIsControlDown)
    {
        if (Key == EKeys::S) // Ctrl+S - Apply
        {
            if (StateManager.IsValid() && StateManager->HasPendingChanges() && !StateManager->IsLoading())
            {
                StateManager->ApplySettings();
                UE_LOG(LogTemp, Log, TEXT("InputHandler: Ctrl+S pressed - Applying settings"));
                return true;
            }
        }
        else if (Key == EKeys::R) // Ctrl+R - Reset current tab
        {
            if (StateManager.IsValid() && TabManager.IsValid() && !StateManager->IsLoading())
            {
                StateManager->ResetCurrentTab(TabManager->GetCurrentTab());
                UE_LOG(LogTemp, Log, TEXT("InputHandler: Ctrl+R pressed - Resetting current tab"));
                return true;
            }
        }
        else if (Key == EKeys::D) // Ctrl+D - Factory defaults
        {
            if (StateManager.IsValid() && !StateManager->IsLoading())
            {
                StateManager->RestoreFactoryDefaults();
                UE_LOG(LogTemp, Log, TEXT("InputHandler: Ctrl+D pressed - Factory defaults"));
                return true;
            }
        }
        else if (Key == EKeys::Z) // Ctrl+Z - Cancel/revert
        {
            if (StateManager.IsValid() && !StateManager->IsLoading())
            {
                StateManager->CancelChanges();
                UE_LOG(LogTemp, Log, TEXT("InputHandler: Ctrl+Z pressed - Canceling changes"));
                return true;
            }
        }
    }
    else
    {
        if (Key == EKeys::Escape) // ESC - Cancel
        {
            if (StateManager.IsValid() && !StateManager->IsLoading())
            {
                StateManager->CancelChanges();
                UE_LOG(LogTemp, Log, TEXT("InputHandler: Escape pressed - Canceling"));
                return true;
            }
        }
        else if (Key == EKeys::Enter || Key == EKeys::SpaceBar) // Enter/Space - Apply
        {
            if (StateManager.IsValid() && StateManager->HasPendingChanges() && !StateManager->IsLoading())
            {
                StateManager->ApplySettings();
                UE_LOG(LogTemp, Log, TEXT("InputHandler: Enter/Space pressed - Applying settings"));
                return true;
            }
        }
    }

    return false;
}

bool USettingsInputHandler::HandleTabNavigation(const FKeyEvent& InKeyEvent)
{
    if (!TabManager.IsValid())
    {
        return false;
    }

    FKey Key = InKeyEvent.GetKey();
    bool bIsShiftDown = InKeyEvent.IsShiftDown();

    if (Key == EKeys::Tab)
    {
        if (bIsShiftDown)
        {
            TabManager->PreviousTab();
            UE_LOG(LogTemp, Log, TEXT("InputHandler: Shift+Tab pressed - Previous tab"));
        }
        else
        {
            TabManager->NextTab();
            UE_LOG(LogTemp, Log, TEXT("InputHandler: Tab pressed - Next tab"));
        }
        return true;
    }
    else if (Key == EKeys::Left || Key == EKeys::A)
    {
        TabManager->PreviousTab();
        UE_LOG(LogTemp, Log, TEXT("InputHandler: Left/A pressed - Previous tab"));
        return true;
    }
    else if (Key == EKeys::Right || Key == EKeys::D)
    {
        TabManager->NextTab();
        UE_LOG(LogTemp, Log, TEXT("InputHandler: Right/D pressed - Next tab"));
        return true;
    }

    return false;
}

bool USettingsInputHandler::HandleNumberShortcuts(const FKeyEvent& InKeyEvent)
{
    if (!TabManager.IsValid())
    {
        return false;
    }

    FKey Key = InKeyEvent.GetKey();

    // Number key shortcuts for direct tab access
    if (Key == EKeys::One || Key == EKeys::NumPadOne)
    {
        TabManager->SwitchToTab(0, true);
        UE_LOG(LogTemp, Log, TEXT("InputHandler: 1 pressed - Switching to Gameplay tab"));
        return true;
    }
    else if (Key == EKeys::Two || Key == EKeys::NumPadTwo)
    {
        TabManager->SwitchToTab(1, true);
        UE_LOG(LogTemp, Log, TEXT("InputHandler: 2 pressed - Switching to Graphics tab"));
        return true;
    }
    else if (Key == EKeys::Three || Key == EKeys::NumPadThree)
    {
        TabManager->SwitchToTab(2, true);
        UE_LOG(LogTemp, Log, TEXT("InputHandler: 3 pressed - Switching to Audio tab"));
        return true;
    }
    else if (Key == EKeys::Four || Key == EKeys::NumPadFour)
    {
        TabManager->SwitchToTab(3, true);
        UE_LOG(LogTemp, Log, TEXT("InputHandler: 4 pressed - Switching to Voice tab"));
        return true;
    }
    else if (Key == EKeys::Five || Key == EKeys::NumPadFive)
    {
        TabManager->SwitchToTab(4, true);
        UE_LOG(LogTemp, Log, TEXT("InputHandler: 5 pressed - Switching to Controls tab"));
        return true;
    }
    else if (Key == EKeys::Six || Key == EKeys::NumPadSix)
    {
        TabManager->SwitchToTab(5, true);
        UE_LOG(LogTemp, Log, TEXT("InputHandler: 6 pressed - Switching to Accessibility tab"));
        return true;
    }

    return false;
}

bool USettingsInputHandler::HandleSpecialKeys(const FKeyEvent& InKeyEvent)
{
    FKey Key = InKeyEvent.GetKey();

    // Handle special function keys
    if (Key == EKeys::F1)
    {
        // Show help/documentation
        UE_LOG(LogTemp, Log, TEXT("InputHandler: F1 pressed - Show help (not implemented)"));
        // TODO: Implement help system
        return true;
    }
    else if (Key == EKeys::F5)
    {
        // Refresh current tab
        if (ParentWidget.IsValid())
        {
            // TODO: Call refresh function on parent widget
            UE_LOG(LogTemp, Log, TEXT("InputHandler: F5 pressed - Refresh current tab"));
        }
        return true;
    }
    else if (Key == EKeys::F11)
    {
        // Toggle fullscreen (if applicable)
        UE_LOG(LogTemp, Log, TEXT("InputHandler: F11 pressed - Toggle fullscreen (not implemented)"));
        return true;
    }

    return false;
}

// === HELPER FUNCTIONS ===

void USettingsInputHandler::InitializeDefaultShortcuts()
{
    KeyboardShortcuts.Empty();

    // Action shortcuts (with Ctrl)
    KeyboardShortcuts.Add(EKeys::S, TEXT("Apply")); // Ctrl+S
    KeyboardShortcuts.Add(EKeys::R, TEXT("Reset")); // Ctrl+R
    KeyboardShortcuts.Add(EKeys::D, TEXT("Defaults")); // Ctrl+D
    KeyboardShortcuts.Add(EKeys::Z, TEXT("Cancel")); // Ctrl+Z

    // Navigation shortcuts
    KeyboardShortcuts.Add(EKeys::Tab, TEXT("NextTab"));
    KeyboardShortcuts.Add(EKeys::Left, TEXT("PreviousTab"));
    KeyboardShortcuts.Add(EKeys::Right, TEXT("NextTab"));
    KeyboardShortcuts.Add(EKeys::A, TEXT("PreviousTab"));

    // Direct tab shortcuts
    KeyboardShortcuts.Add(EKeys::One, TEXT("Tab0"));
    KeyboardShortcuts.Add(EKeys::Two, TEXT("Tab1"));
    KeyboardShortcuts.Add(EKeys::Three, TEXT("Tab2"));
    KeyboardShortcuts.Add(EKeys::Four, TEXT("Tab3"));
    KeyboardShortcuts.Add(EKeys::Five, TEXT("Tab4"));
    KeyboardShortcuts.Add(EKeys::Six, TEXT("Tab5"));

    // Special keys
    KeyboardShortcuts.Add(EKeys::Escape, TEXT("Cancel"));
    KeyboardShortcuts.Add(EKeys::Enter, TEXT("Apply"));
    KeyboardShortcuts.Add(EKeys::SpaceBar, TEXT("Apply"));
    KeyboardShortcuts.Add(EKeys::F1, TEXT("Help"));
    KeyboardShortcuts.Add(EKeys::F5, TEXT("Refresh"));
    KeyboardShortcuts.Add(EKeys::F11, TEXT("Fullscreen"));

    UE_LOG(LogTemp, Log, TEXT("SettingsInputHandler: %d keyboard shortcuts initialized"), KeyboardShortcuts.Num());
}

bool USettingsInputHandler::ExecuteShortcut(const FString& ShortcutName)
{
    UE_LOG(LogTemp, Log, TEXT("SettingsInputHandler: Executing shortcut - %s"), *ShortcutName);

    // Execute the appropriate action based on shortcut name
    if (ShortcutName == TEXT("Apply") && StateManager.IsValid())
    {
        if (StateManager->HasPendingChanges() && !StateManager->IsLoading())
        {
            StateManager->ApplySettings();
            return true;
        }
    }
    else if (ShortcutName == TEXT("Reset") && StateManager.IsValid() && TabManager.IsValid())
    {
        if (!StateManager->IsLoading())
        {
            StateManager->ResetCurrentTab(TabManager->GetCurrentTab());
            return true;
        }
    }
    else if (ShortcutName == TEXT("Defaults") && StateManager.IsValid())
    {
        if (!StateManager->IsLoading())
        {
            StateManager->RestoreFactoryDefaults();
            return true;
        }
    }
    else if (ShortcutName == TEXT("Cancel") && StateManager.IsValid())
    {
        if (!StateManager->IsLoading())
        {
            StateManager->CancelChanges();
            return true;
        }
    }
    else if (ShortcutName == TEXT("NextTab") && TabManager.IsValid())
    {
        TabManager->NextTab();
        return true;
    }
    else if (ShortcutName == TEXT("PreviousTab") && TabManager.IsValid())
    {
        TabManager->PreviousTab();
        return true;
    }
    else if (ShortcutName.StartsWith(TEXT("Tab")) && TabManager.IsValid())
    {
        // Extract tab number
        FString TabNumberStr = ShortcutName.RightChop(3); // Remove "Tab" prefix
        int32 TabIndex = FCString::Atoi(*TabNumberStr);
        if (TabIndex >= 0 && TabIndex < 6) // Validate tab index
        {
            TabManager->SwitchToTab(TabIndex, true);
            return true;
        }
    }
    else if (ShortcutName == TEXT("Help"))
    {
        UE_LOG(LogTemp, Log, TEXT("Help shortcut executed - Help system not implemented"));
        return true;
    }
    else if (ShortcutName == TEXT("Refresh"))
    {
        UE_LOG(LogTemp, Log, TEXT("Refresh shortcut executed - Refresh functionality not implemented"));
        return true;
    }
    else if (ShortcutName == TEXT("Fullscreen"))
    {
        UE_LOG(LogTemp, Log, TEXT("Fullscreen shortcut executed - Fullscreen toggle not implemented"));
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("SettingsInputHandler::ExecuteShortcut - Unknown or unavailable shortcut: %s"), *ShortcutName);
    return false;
}

bool USettingsInputHandler::IsModifierKeyDown(const FKeyEvent& InKeyEvent, const FString& ModifierName) const
{
    if (ModifierName == TEXT("Ctrl"))
    {
        return InKeyEvent.IsControlDown();
    }
    else if (ModifierName == TEXT("Shift"))
    {
        return InKeyEvent.IsShiftDown();
    }
    else if (ModifierName == TEXT("Alt"))
    {
        return InKeyEvent.IsAltDown();
    }
    else if (ModifierName == TEXT("Cmd"))
    {
        return InKeyEvent.IsCommandDown();
    }

    return false;
}

// === DEBUG FUNCTIONS ===

void USettingsInputHandler::DebugPrintShortcuts() const
{
    UE_LOG(LogTemp, Log, TEXT("=== SettingsInputHandler Shortcuts Debug ==="));
    UE_LOG(LogTemp, Log, TEXT("Shortcuts Enabled: %s"), bKeyboardShortcutsEnabled ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("Total Shortcuts: %d"), KeyboardShortcuts.Num());
    UE_LOG(LogTemp, Log, TEXT("TabManager Valid: %s"), TabManager.IsValid() ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("StateManager Valid: %s"), StateManager.IsValid() ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("ParentWidget Valid: %s"), ParentWidget.IsValid() ? TEXT("true") : TEXT("false"));

    UE_LOG(LogTemp, Log, TEXT("--- Shortcut Mappings ---"));
    for (const auto& Shortcut : KeyboardShortcuts)
    {
        UE_LOG(LogTemp, Log, TEXT("Key: %s -> Action: %s"),
            *Shortcut.Key.ToString(), *Shortcut.Value);
    }

    UE_LOG(LogTemp, Log, TEXT("--- Available Shortcuts ---"));
    UE_LOG(LogTemp, Log, TEXT("Ctrl+S: Apply Settings"));
    UE_LOG(LogTemp, Log, TEXT("Ctrl+R: Reset Current Tab"));
    UE_LOG(LogTemp, Log, TEXT("Ctrl+D: Factory Defaults"));
    UE_LOG(LogTemp, Log, TEXT("Ctrl+Z: Cancel Changes"));
    UE_LOG(LogTemp, Log, TEXT("Tab/Shift+Tab: Navigate Tabs"));
    UE_LOG(LogTemp, Log, TEXT("Left/Right Arrow: Navigate Tabs"));
    UE_LOG(LogTemp, Log, TEXT("A/D: Navigate Tabs"));
    UE_LOG(LogTemp, Log, TEXT("1-6: Direct Tab Access"));
    UE_LOG(LogTemp, Log, TEXT("Enter/Space: Apply Settings"));
    UE_LOG(LogTemp, Log, TEXT("Escape: Cancel"));
    UE_LOG(LogTemp, Log, TEXT("F1: Help"));
    UE_LOG(LogTemp, Log, TEXT("F5: Refresh"));
    UE_LOG(LogTemp, Log, TEXT("F11: Toggle Fullscreen"));
    UE_LOG(LogTemp, Log, TEXT("=== End Shortcuts Debug ==="));
}