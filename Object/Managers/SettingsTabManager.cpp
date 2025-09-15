#include "SettingsTabManager.h"
#include "HorrorGame/Widget/Settings/MainSettings/MenuSettingSystemWidget.h"
#include "HorrorGame/Object/Managers/SettingsAudioManager.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"

USettingsTabManager::USettingsTabManager()
    : CurrentTab(0)
    , ParentWidget(nullptr)
    , AudioManager(nullptr)
    , ContentSwitcher(nullptr)
    , TabDescriptionText(nullptr)
    , SelectedTabColor(0.2f, 0.6f, 1.0f, 1.0f)
    , UnselectedTabColor(0.15f, 0.15f, 0.15f, 0.9f)
    , SelectedTabScale(1.02f, 1.02f)
    , NormalTabScale(1.0f, 1.0f)
{
}

void USettingsTabManager::DebugPrintTabInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== SettingsTabManager Debug Info ==="));
    UE_LOG(LogTemp, Log, TEXT("TabData: %d"), TabData.Num());
    UE_LOG(LogTemp, Log, TEXT("TabButtons: %d"), TabButtons.Num());
    UE_LOG(LogTemp, Log, TEXT("CurrentTab: %d"), CurrentTab);
    UE_LOG(LogTemp, Log, TEXT("ParentWidget: %s"), ParentWidget.IsValid() ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("AudioManager: %s"), AudioManager.IsValid() ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("ContentSwitcher: %s"), ContentSwitcher.IsValid() ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("TabDescriptionText: %s"), TabDescriptionText.IsValid() ? TEXT("Valid") : TEXT("NULL"));

    if (ContentSwitcher.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("ContentSwitcher Children Count: %d"), ContentSwitcher->GetChildrenCount());
        UE_LOG(LogTemp, Log, TEXT("ContentSwitcher Active Widget Index: %d"), ContentSwitcher->GetActiveWidgetIndex());

        UWidget* ActiveWidget = ContentSwitcher->GetActiveWidget();
        UE_LOG(LogTemp, Log, TEXT("Active Widget: %s"), ActiveWidget ? *ActiveWidget->GetClass()->GetName() : TEXT("NULL"));

        // Debug: Print all children
        for (int32 i = 0; i < ContentSwitcher->GetChildrenCount(); ++i)
        {
            UWidget* Child = ContentSwitcher->GetChildAt(i);
            UE_LOG(LogTemp, Log, TEXT("Child[%d]: %s"), i, Child ? *Child->GetClass()->GetName() : TEXT("NULL"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("=== End SettingsTabManager Debug Info ==="));
}

void USettingsTabManager::DebugWidgetVisibility(int32 TabIndex)
{
    if (!ContentSwitcher.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ContentSwitcher is invalid"));
        return;
    }

    UWidget* Widget = ContentSwitcher->GetChildAt(TabIndex);
    if (!Widget)
    {
        UE_LOG(LogTemp, Error, TEXT("Widget at index %d is null"), TabIndex);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("=== Widget Visibility Debug for Index %d ==="), TabIndex);
    UE_LOG(LogTemp, Warning, TEXT("Widget Class: %s"), *Widget->GetClass()->GetName());
    UE_LOG(LogTemp, Warning, TEXT("Widget Visibility: %s"),
        Widget->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") :
        Widget->GetVisibility() == ESlateVisibility::Collapsed ? TEXT("Collapsed") :
        Widget->GetVisibility() == ESlateVisibility::Hidden ? TEXT("Hidden") :
        Widget->GetVisibility() == ESlateVisibility::HitTestInvisible ? TEXT("HitTestInvisible") :
        Widget->GetVisibility() == ESlateVisibility::SelfHitTestInvisible ? TEXT("SelfHitTestInvisible") : TEXT("Unknown"));

    UE_LOG(LogTemp, Warning, TEXT("Widget IsVisible: %s"), Widget->IsVisible() ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("Widget RenderOpacity: %f"), Widget->GetRenderOpacity());
    UE_LOG(LogTemp, Warning, TEXT("Widget DesiredSize: %s"), *Widget->GetDesiredSize().ToString());
    //UE_LOG(LogTemp, Warning, TEXT("Widget CachedGeometry IsValid: %s"), Widget->GetCachedGeometry()? TEXT("true") : TEXT("false"));

    // Check ContentSwitcher itself
    UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher Visibility: %s"),
        ContentSwitcher->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("Not Visible"));
    UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher IsVisible: %s"), ContentSwitcher->IsVisible() ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher DesiredSize: %s"), *ContentSwitcher->GetDesiredSize().ToString());

    // Force refresh
    Widget->SetVisibility(ESlateVisibility::Visible);
    Widget->InvalidateLayoutAndVolatility();
    Widget->ForceLayoutPrepass();
    ContentSwitcher->InvalidateLayoutAndVolatility();

    UE_LOG(LogTemp, Warning, TEXT("=== End Widget Visibility Debug ==="));
}

void USettingsTabManager::Initialize(UMenuSettingSystemWidget* InParentWidget)
{
    if (!InParentWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsTabManager::Initialize - ParentWidget is null!"));
        return;
    }

    ParentWidget = InParentWidget;

    // Cache UI references (assume these getter functions exist in MenuSettingSystemWidget)
    ContentSwitcher = InParentWidget->GetContentSwitcher();
    TabDescriptionText = InParentWidget->GetTabDescriptionText();

    if (!ContentSwitcher.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsTabManager::Initialize - ContentSwitcher is null!"));
        return;
    }

    // Initialize tab data
    InitializeTabData();

    // Populate tab buttons (assume these getter functions exist)
    TabButtons.Empty();
    TabButtons.Add(InParentWidget->GetGameplayTabButton());
    TabButtons.Add(InParentWidget->GetGraphicsTabButton());
    TabButtons.Add(InParentWidget->GetAudioTabButton());
    TabButtons.Add(InParentWidget->GetVoiceTabButton());
    TabButtons.Add(InParentWidget->GetControlsTabButton());
    TabButtons.Add(InParentWidget->GetAccessibilityTabButton());

    // Validate all buttons exist
    bool bAllButtonsValid = true;
    for (int32 i = 0; i < TabButtons.Num(); ++i)
    {
        if (!TabButtons[i])
        {
            UE_LOG(LogTemp, Error, TEXT("Tab button at index %d is null!"), i);
            bAllButtonsValid = false;
        }
    }

    if (!bAllButtonsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Some tab buttons are null - check MenuSettingSystemWidget getter functions"));
        return;
    }

    // Bind events
    BindTabButtons();

    // Wait for widget factory to create widgets before setting initial state
    // This will be called after widget factory initialization
    UE_LOG(LogTemp, Log, TEXT("SettingsTabManager initialized with %d tabs"), TabData.Num());
}

void USettingsTabManager::SetInitialTab()
{
    // This should be called AFTER WidgetFactory creates all widgets
    if (ContentSwitcher.IsValid() && ContentSwitcher->GetChildrenCount() > 0)
    {
        SwitchToTab(0, false);
        UE_LOG(LogTemp, Log, TEXT("Initial tab set to 0"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot set initial tab - ContentSwitcher has no children"));
    }
}

void USettingsTabManager::InitializeTabData()
{
    TabData.Empty();
    TabData.Add(FTabInfo(TEXT("Gameplay"), TEXT("Game mechanics and difficulty settings"), nullptr, 0));
    TabData.Add(FTabInfo(TEXT("Graphics"), TEXT("Visual quality and display options"), nullptr, 1));
    TabData.Add(FTabInfo(TEXT("Audio"), TEXT("Sound and music volume controls"), nullptr, 2));
    TabData.Add(FTabInfo(TEXT("Voice"), TEXT("Voice chat and communication settings"), nullptr, 3));
    TabData.Add(FTabInfo(TEXT("Controls"), TEXT("Input and keybinding configuration"), nullptr, 4));
    TabData.Add(FTabInfo(TEXT("Accessibility"), TEXT("Options for enhanced accessibility"), nullptr, 5));
}

void USettingsTabManager::BindTabButtons()
{
    if (TabButtons.Num() < 6)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsTabManager::BindTabButtons - Not enough tab buttons! Found: %d, Expected: 6"), TabButtons.Num());
        return;
    }

    if (TabButtons[0]) // Gameplay
    {
        TabButtons[0]->OnClicked.AddDynamic(this, &USettingsTabManager::OnGameplayTabClicked);
        TabButtons[0]->OnHovered.AddDynamic(this, &USettingsTabManager::OnTabButtonHovered);
        TabButtons[0]->OnUnhovered.AddDynamic(this, &USettingsTabManager::OnTabButtonUnhovered);
    }

    if (TabButtons[1]) // Graphics
    {
        TabButtons[1]->OnClicked.AddDynamic(this, &USettingsTabManager::OnGraphicsTabClicked);
        TabButtons[1]->OnHovered.AddDynamic(this, &USettingsTabManager::OnTabButtonHovered);
        TabButtons[1]->OnUnhovered.AddDynamic(this, &USettingsTabManager::OnTabButtonUnhovered);
    }

    if (TabButtons[2]) // Audio
    {
        TabButtons[2]->OnClicked.AddDynamic(this, &USettingsTabManager::OnAudioTabClicked);
        TabButtons[2]->OnHovered.AddDynamic(this, &USettingsTabManager::OnTabButtonHovered);
        TabButtons[2]->OnUnhovered.AddDynamic(this, &USettingsTabManager::OnTabButtonUnhovered);
    }

    if (TabButtons[3]) // Voice
    {
        TabButtons[3]->OnClicked.AddDynamic(this, &USettingsTabManager::OnVoiceTabClicked);
        TabButtons[3]->OnHovered.AddDynamic(this, &USettingsTabManager::OnTabButtonHovered);
        TabButtons[3]->OnUnhovered.AddDynamic(this, &USettingsTabManager::OnTabButtonUnhovered);
    }

    if (TabButtons[4]) // Controls
    {
        TabButtons[4]->OnClicked.AddDynamic(this, &USettingsTabManager::OnControlsTabClicked);
        TabButtons[4]->OnHovered.AddDynamic(this, &USettingsTabManager::OnTabButtonHovered);
        TabButtons[4]->OnUnhovered.AddDynamic(this, &USettingsTabManager::OnTabButtonUnhovered);
    }

    if (TabButtons[5]) // Accessibility
    {
        TabButtons[5]->OnClicked.AddDynamic(this, &USettingsTabManager::OnAccessibilityTabClicked);
        TabButtons[5]->OnHovered.AddDynamic(this, &USettingsTabManager::OnTabButtonHovered);
        TabButtons[5]->OnUnhovered.AddDynamic(this, &USettingsTabManager::OnTabButtonUnhovered);
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsTabManager: Tab buttons bound successfully"));
}

void USettingsTabManager::SwitchToTab(int32 TabIndex, bool bPlaySound)
{
    UE_LOG(LogTemp, Warning, TEXT("SwitchToTab called: TabIndex=%d, CurrentTab=%d"), TabIndex, CurrentTab);

    if (!ContentSwitcher.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SwitchToTab: ContentSwitcher is invalid!"));
        return;
    }

    if (!IsValidTabIndex(TabIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("SwitchToTab: Invalid TabIndex %d (Valid range: 0-%d)"), TabIndex, TabData.Num() - 1);
        return;
    }

    int32 ChildCount = ContentSwitcher->GetChildrenCount();
    if (ChildCount == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SwitchToTab: ContentSwitcher has no children! Widgets may not be created yet."));
        return;
    }

    if (TabIndex >= ChildCount)
    {
        UE_LOG(LogTemp, Error, TEXT("SwitchToTab: TabIndex %d out of range (ContentSwitcher children: %d)"), TabIndex, ChildCount);
        return;
    }

    if (CurrentTab == TabIndex)
    {
        UE_LOG(LogTemp, Log, TEXT("SwitchToTab: Already on tab %d, but forcing refresh"), TabIndex);
        // Don't return early - force refresh the current tab
    }

    int32 PreviousTab = CurrentTab;
    CurrentTab = TabIndex;

    // Play sound
    if (bPlaySound && AudioManager.IsValid())
    {
        AudioManager->PlaySound(ESoundTypes::TabSwitch);
    }

    UE_LOG(LogTemp, Warning, TEXT("=== BEFORE WIDGET SWITCHING ==="));
    UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher children: %d"), ChildCount);
    UE_LOG(LogTemp, Warning, TEXT("Attempting to switch to index: %d"), TabIndex);

    // Log current state of all children
    for (int32 i = 0; i < ChildCount; ++i)
    {
        UWidget* Child = ContentSwitcher->GetChildAt(i);
        if (Child)
        {
            UE_LOG(LogTemp, Warning, TEXT("Child[%d] (%s): Visibility=%s, IsVisible=%s"),
                i, *Child->GetClass()->GetName(),
                Child->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("NotVisible"),
                Child->IsVisible() ? TEXT("true") : TEXT("false"));
        }
    }

    // CRITICAL FIX: Force hide all widgets first
    for (int32 i = 0; i < ChildCount; ++i)
    {
        UWidget* Child = ContentSwitcher->GetChildAt(i);
        if (Child)
        {
            Child->SetVisibility(ESlateVisibility::Collapsed);
            Child->SetRenderOpacity(0.0f);
        }
    }

    // CRITICAL FIX: Set active widget index BEFORE showing the widget
    ContentSwitcher->SetActiveWidgetIndex(TabIndex);

    // CRITICAL FIX: Get the active widget and force it to be visible
    UWidget* ActiveWidget = ContentSwitcher->GetChildAt(TabIndex);
    if (ActiveWidget)
    {
        // Force visibility and opacity
        ActiveWidget->SetVisibility(ESlateVisibility::Visible);
        ActiveWidget->SetRenderOpacity(1.0f);

        // Force layout updates
        ActiveWidget->InvalidateLayoutAndVolatility();
        ActiveWidget->ForceLayoutPrepass();

        // Force content switcher to update
        ContentSwitcher->InvalidateLayoutAndVolatility();
        ContentSwitcher->ForceLayoutPrepass();

        UE_LOG(LogTemp, Warning, TEXT("=== AFTER WIDGET SWITCHING ==="));
        UE_LOG(LogTemp, Warning, TEXT("Active widget set to: %s at index %d"), *ActiveWidget->GetClass()->GetName(), TabIndex);
        UE_LOG(LogTemp, Warning, TEXT("Active widget visibility: %s"), ActiveWidget->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("NotVisible"));
        UE_LOG(LogTemp, Warning, TEXT("Active widget IsVisible: %s"), ActiveWidget->IsVisible() ? TEXT("true") : TEXT("false"));
        UE_LOG(LogTemp, Warning, TEXT("Active widget opacity: %f"), ActiveWidget->GetRenderOpacity());
        UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher active index: %d"), ContentSwitcher->GetActiveWidgetIndex());

        // Additional debug: Check if the widget has content
        if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(ActiveWidget))
        {
            UE_LOG(LogTemp, Warning, TEXT("Active widget has %d children"), PanelWidget->GetChildrenCount());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SwitchToTab: Failed to get widget at index %d"), TabIndex);
        return;
    }

    // Force ContentSwitcher refresh
    ContentSwitcher->SetVisibility(ESlateVisibility::Visible);
    ContentSwitcher->SetRenderOpacity(1.0f);

    // Animate transition
    if (PreviousTab != CurrentTab)
    {
        AnimateTabTransition(PreviousTab, CurrentTab);
    }

    // Update UI states
    UpdateAllTabStates();
    UpdateTabDescription(CurrentTab);

    // Broadcast event
    OnTabChanged.Broadcast(CurrentTab, PreviousTab);

    UE_LOG(LogTemp, Warning, TEXT("SettingsTabManager: Successfully switched to tab %d (%s)"),
        CurrentTab, TabData.IsValidIndex(CurrentTab) ? *TabData[CurrentTab].Name : TEXT("Unknown"));
}

UButton* USettingsTabManager::GetTabButton(int32 Index) const
{
    if (TabButtons.IsValidIndex(Index))
    {
        return TabButtons[Index];
    }
    return nullptr;
}

void USettingsTabManager::NextTab()
{
    int32 NextIndex = FindNextValidTab(CurrentTab, true);
    if (NextIndex != CurrentTab)
    {
        SwitchToTab(NextIndex, true);
    }
}

void USettingsTabManager::PreviousTab()
{
    int32 PrevIndex = FindNextValidTab(CurrentTab, false);
    if (PrevIndex != CurrentTab)
    {
        SwitchToTab(PrevIndex, true);
    }
}

void USettingsTabManager::UpdateAllTabStates()
{
    for (int32 i = 0; i < TabButtons.Num(); ++i)
    {
        if (TabButtons[i])
        {
            SetTabButtonStyle(TabButtons[i], i == CurrentTab, i);
        }
    }
}

void USettingsTabManager::SetTabButtonStyle(UButton* Button, bool bSelected, int32 TabIndex)
{
    if (!Button || !IsValidTabIndex(TabIndex))
        return;

    FLinearColor ButtonColor = bSelected ? SelectedTabColor : UnselectedTabColor;
    FVector2D ButtonScale = bSelected ? SelectedTabScale : NormalTabScale;

    Button->SetBackgroundColor(ButtonColor);
    Button->SetRenderScale(ButtonScale);

    // Update border if exists
    if (UBorder* ButtonBorder = Cast<UBorder>(Button->GetChildAt(0)))
    {
        FLinearColor BorderColor = bSelected ?
            FLinearColor(0.3f, 0.7f, 1.0f, 0.8f) :
            FLinearColor(0.2f, 0.2f, 0.2f, 0.7f);
        ButtonBorder->SetBrushColor(BorderColor);
    }

    // Update icon if exists
    if (TabData[TabIndex].IconWidget)
    {
        FLinearColor IconColor = bSelected ?
            FLinearColor::White :
            FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
        TabData[TabIndex].IconWidget->SetColorAndOpacity(IconColor);
    }
}

void USettingsTabManager::AnimateTabTransition(int32 FromTab, int32 ToTab)
{
    if (AudioManager.IsValid())
    {
        AudioManager->PlayTabTransition();
    }
}

bool USettingsTabManager::IsValidTabIndex(int32 TabIndex) const
{
    return TabIndex >= 0 && TabIndex < TabData.Num();
}

void USettingsTabManager::SetAudioManager(USettingsAudioManager* InAudioManager)
{
    AudioManager = InAudioManager;
}

// === EVENT HANDLERS ===

void USettingsTabManager::OnGameplayTabClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Gameplay tab clicked"));
    SwitchToTab(0, true);
}

void USettingsTabManager::OnGraphicsTabClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Graphics tab clicked"));
    SwitchToTab(1, true);
}

void USettingsTabManager::OnAudioTabClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Audio tab clicked"));
    SwitchToTab(2, true);
}

void USettingsTabManager::OnVoiceTabClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Voice tab clicked"));
    SwitchToTab(3, true);
}

void USettingsTabManager::OnControlsTabClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Controls tab clicked"));
    SwitchToTab(4, true);
}

void USettingsTabManager::OnAccessibilityTabClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Accessibility tab clicked"));
    SwitchToTab(5, true);
}

void USettingsTabManager::OnTabButtonHovered()
{
    if (AudioManager.IsValid())
    {
        AudioManager->PlaySound(ESoundTypes::Hover);
    }
}

void USettingsTabManager::OnTabButtonUnhovered()
{
    // Optional: Add unhover effects
}

// === HELPER FUNCTIONS ===

void USettingsTabManager::UpdateTabDescription(int32 TabIndex)
{
    if (TabDescriptionText.IsValid() && IsValidTabIndex(TabIndex))
    {
        TabDescriptionText->SetText(FText::FromString(TabData[TabIndex].Description));
    }
}

int32 USettingsTabManager::FindNextValidTab(int32 StartIndex, bool bForward) const
{
    if (TabData.Num() == 0) return 0;

    int32 Direction = bForward ? 1 : -1;
    int32 CheckIndex = (StartIndex + Direction + TabData.Num()) % TabData.Num();
    int32 OriginalIndex = StartIndex;

    // Find next enabled tab
    while (CheckIndex != OriginalIndex)
    {
        if (TabData[CheckIndex].bIsEnabled)
        {
            return CheckIndex;
        }
        CheckIndex = (CheckIndex + Direction + TabData.Num()) % TabData.Num();
    }

    return StartIndex; // No valid tab found, stay on current
}