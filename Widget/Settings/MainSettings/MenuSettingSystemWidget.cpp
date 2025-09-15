#include "MenuSettingSystemWidget.h"
#include "HorrorGame/Object/Managers/SettingsTabManager.h"
#include "HorrorGame/Object/Managers/SettingsStateManager.h"
#include "HorrorGame/Object/Managers/SettingsAudioManager.h"
#include "HorrorGame/Object/Managers/SettingsWidgetFactory.h"
#include "HorrorGame/Object/Managers/SettingsUIManager.h"
#include "HorrorGame/Object/Managers/SettingsInputHandler.h"
#include "HorrorGame/Widget/Settings/SettingsTab/GameplayWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/GraphicsWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/AudioWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/Voice/VoiceWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/ControlsWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/AccessibilityWidget.h"

UMenuSettingSystemWidget::UMenuSettingSystemWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , TabManager(nullptr)
    , StateManager(nullptr)
    , AudioManager(nullptr)
    , WidgetFactory(nullptr)
    , UIManager(nullptr)
    , InputHandler(nullptr)
{
}

void UMenuSettingSystemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget::NativeConstruct - Starting initialization"));

    // Create all managers first
    CreateManagers();

    // Initialize managers in dependency order
    InitializeManagers();

    // Bind events between managers
    BindManagerEvents();

    // Bind UI button events
    BindButtonEvents();

    // Setup audio manager with assets
    SetupAudioManager();

    // DEBUG: Print widget hierarchy before setting initial tab
    DebugWidgetHierarchy();

    // CRITICAL FIX: Ensure widgets are created before setting initial tab
    if (WidgetFactory && ContentSwitcher)
    {
        // Force widget creation if not already done
        if (ContentSwitcher->GetChildrenCount() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher is empty, forcing widget creation"));
            WidgetFactory->CreateAllChildWidgets();
        }

        // Verify widgets were created
        if (ContentSwitcher->GetChildrenCount() > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("ContentSwitcher has %d children, setting initial tab"), ContentSwitcher->GetChildrenCount());

            // Set initial tab immediately instead of using timer
            if (TabManager)
            {
                TabManager->SetInitialTab();
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ContentSwitcher still empty after widget creation attempt!"));
        }
    }

    // Play fade-in animation
    if (AudioManager && FadeInAnimation)
    {
        AudioManager->PlayFadeIn();
    }

    // Debug output
    if (WidgetFactory) WidgetFactory->DebugPrintWidgetInfo();
    if (TabManager) TabManager->DebugPrintTabInfo();

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: Fully initialized with all managers"));
}

void UMenuSettingSystemWidget::NativeDestruct()
{
    // Cleanup is handled automatically by UPROPERTY garbage collection
    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: Destructed"));
    Super::NativeDestruct();
}

void UMenuSettingSystemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    // Most tick functionality moved to managers for performance
}

UWidget* UMenuSettingSystemWidget::NativeGetDesiredFocusTarget() const
{
    // Delegate to tab manager for focus handling
    if (TabManager)
    {
        UButton* FirstTabButton = TabManager->GetTabButton(0);
        if (FirstTabButton)
        {
            return FirstTabButton;
        }
    }

    return GameplayTabButton ? GameplayTabButton : Super::NativeGetDesiredFocusTarget();
}

FReply UMenuSettingSystemWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Delegate all input handling to InputHandler
    if (InputHandler)
    {
        FReply Result = InputHandler->HandleKeyDown(InKeyEvent);
        if (Result.IsEventHandled())
        {
            return Result;
        }
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// === MANAGER CREATION AND INITIALIZATION ===

void UMenuSettingSystemWidget::CreateManagers()
{
    // Create all manager objects
    TabManager = NewObject<USettingsTabManager>(this);
    StateManager = NewObject<USettingsStateManager>(this);
    AudioManager = NewObject<USettingsAudioManager>(this);
    WidgetFactory = NewObject<USettingsWidgetFactory>(this);
    UIManager = NewObject<USettingsUIManager>(this);
    InputHandler = NewObject<USettingsInputHandler>(this);

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: All managers created"));
}

void UMenuSettingSystemWidget::InitializeManagers()
{
    // Initialize managers in the CORRECT dependency order

    UE_LOG(LogTemp, Log, TEXT("Initializing managers..."));

    // 1. Audio Manager first (no dependencies)
    if (AudioManager)
    {
        AudioManager->SetParentWidget(this);
        AudioManager->SetAnimations(FadeInAnimation, FadeOutAnimation, TabTransitionAnimation, ButtonHoverAnimation);
        UE_LOG(LogTemp, Log, TEXT("AudioManager initialized"));
    }

    // 2. UI Manager (needs widget references)
    if (UIManager)
    {
        UIManager->Initialize(this);
        UE_LOG(LogTemp, Log, TEXT("UIManager initialized"));
    }

    // 3. CRITICAL FIX: WIDGET FACTORY FIRST AND VERIFY CONTENT SWITCHER
    if (WidgetFactory)
    {
        // Ensure ContentSwitcher is available before initializing widget factory
        if (!ContentSwitcher)
        {
            UE_LOG(LogTemp, Error, TEXT("ContentSwitcher is null in InitializeManagers! Check Blueprint bindings!"));
            return;
        }

        WidgetFactory->Initialize(this);
        UE_LOG(LogTemp, Log, TEXT("WidgetFactory initialized"));

        // FORCE IMMEDIATE WIDGET CREATION
        WidgetFactory->CreateAllChildWidgets();
        UE_LOG(LogTemp, Log, TEXT("Forced widget creation complete"));
    }

    // 4. State Manager (needs widget references)
    if (StateManager)
    {
        StateManager->Initialize(this);
        UE_LOG(LogTemp, Log, TEXT("StateManager initialized"));
    }

    // 5. Tab Manager AFTER widget factory (needs widgets to exist)
    if (TabManager)
    {
        TabManager->Initialize(this);
        TabManager->SetAudioManager(AudioManager);
        UE_LOG(LogTemp, Log, TEXT("TabManager initialized"));
    }

    // 6. Input Handler last (needs manager references)
    if (InputHandler)
    {
        InputHandler->Initialize(this);
        InputHandler->SetTabManager(TabManager);
        InputHandler->SetStateManager(StateManager);
        UE_LOG(LogTemp, Log, TEXT("InputHandler initialized"));
    }

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: All managers initialized"));
}

void UMenuSettingSystemWidget::BindManagerEvents()
{
    // Bind Tab Manager events
    if (TabManager)
    {
        TabManager->OnTabChanged.AddDynamic(this, &UMenuSettingSystemWidget::OnManagerTabChanged);
    }

    // Bind State Manager events
    if (StateManager)
    {
        StateManager->OnSettingsApplied.AddDynamic(this, &UMenuSettingSystemWidget::OnManagerSettingsApplied);
        StateManager->OnSettingsReset.AddDynamic(this, &UMenuSettingSystemWidget::OnManagerSettingsReset);
    }

    // Bind Audio Manager events
    if (AudioManager)
    {
        AudioManager->OnFadeOutCompleted.AddDynamic(this, &UMenuSettingSystemWidget::OnManagerFadeOutCompleted);
    }

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: Manager events bound"));
}

void UMenuSettingSystemWidget::BindButtonEvents()
{
    // Bind action button events - these just delegate to managers
    if (ApplyButton)
    {
        ApplyButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnApplyClicked);
    }

    if (ResetButton)
    {
        ResetButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnResetClicked);
    }

    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnBackClicked);
    }

    if (DefaultsButton)
    {
        DefaultsButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnDefaultsClicked);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UMenuSettingSystemWidget::OnCancelClicked);
    }

    // Tab buttons are bound automatically by TabManager
    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: Button events bound"));
}

void UMenuSettingSystemWidget::SetupAudioManager()
{
    if (!AudioManager)
    {
        return;
    }

    // Set all sound cues
    AudioManager->SetSoundCues(
        TabSwitchSound,
        HoverSound,
        ApplySound,
        ResetSound,
        BackSound,
        ErrorSound,
        SettingsChangedSound
    );

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: Audio manager setup complete"));
}

// === PUBLIC INTERFACE (delegates to managers) ===

void UMenuSettingSystemWidget::SwitchToTab(int32 TabIndex)
{
    if (TabManager)
    {
        TabManager->SwitchToTab(TabIndex, true);
    }
}

void UMenuSettingSystemWidget::MarkSettingsChanged()
{
    if (StateManager)
    {
        StateManager->MarkSettingsChanged();
    }
}

bool UMenuSettingSystemWidget::HasPendingChanges() const
{
    return StateManager ? StateManager->HasPendingChanges() : false;
}

int32 UMenuSettingSystemWidget::GetCurrentTab() const
{
    return TabManager ? TabManager->GetCurrentTab() : 0;
}

void UMenuSettingSystemWidget::ShowCustomNotification(const FString& Message, ENotificationType Type)
{
    if (UIManager)
    {
        UIManager->ShowNotification(Message, Type);
    }
}

// === CHILD WIDGET ACCESS (delegates to factory) ===

UGameplayWidget* UMenuSettingSystemWidget::GetGameplayWidget() const
{
    return WidgetFactory ? WidgetFactory->GetGameplayWidget() : nullptr;
}

UGraphicsWidget* UMenuSettingSystemWidget::GetGraphicsWidget() const
{
    return WidgetFactory ? WidgetFactory->GetGraphicsWidget() : nullptr;
}

UAudioWidget* UMenuSettingSystemWidget::GetAudioWidget() const
{
    return WidgetFactory ? WidgetFactory->GetAudioWidget() : nullptr;
}

UVoiceWidget* UMenuSettingSystemWidget::GetVoiceWidget() const
{
    return WidgetFactory ? WidgetFactory->GetVoiceWidget() : nullptr;
}

UControlsWidget* UMenuSettingSystemWidget::GetControlsWidget() const
{
    return WidgetFactory ? WidgetFactory->GetControlsWidget() : nullptr;
}

UAccessibilityWidget* UMenuSettingSystemWidget::GetAccessibilityWidget() const
{
    return WidgetFactory ? WidgetFactory->GetAccessibilityWidget() : nullptr;
}

// === SIMPLE EVENT HANDLERS (delegate to managers) ===

void UMenuSettingSystemWidget::OnApplyClicked()
{
    if (StateManager)
    {
        StateManager->ApplySettings();
    }
}

void UMenuSettingSystemWidget::OnResetClicked()
{
    if (StateManager && TabManager)
    {
        StateManager->ResetCurrentTab(TabManager->GetCurrentTab());
    }
}

void UMenuSettingSystemWidget::OnBackClicked()
{
    // Play back sound
    if (AudioManager)
    {
        AudioManager->PlaySound(ESoundTypes::Back);
    }

    // Start fade out animation which will trigger removal
    if (AudioManager)
    {
        AudioManager->PlayFadeOut();
    }
    else
    {
        // Fallback if no audio manager
        OnManagerFadeOutCompleted();
    }
}

void UMenuSettingSystemWidget::OnDefaultsClicked()
{
    if (StateManager)
    {
        StateManager->RestoreFactoryDefaults();
    }
}

void UMenuSettingSystemWidget::OnCancelClicked()
{
    if (StateManager)
    {
        StateManager->CancelChanges();
    }
}

// === MANAGER EVENT HANDLERS ===

void UMenuSettingSystemWidget::OnManagerTabChanged(int32 NewTab, int32 OldTab)
{
    // Update button states when tab changes
    if (UIManager && StateManager)
    {
        UIManager->UpdateActionButtonStates(StateManager->HasPendingChanges(), StateManager->IsLoading());
    }

    // Broadcast to Blueprint
    OnTabChangedDelegate.Broadcast(NewTab, OldTab);

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: Tab changed from %d to %d"), OldTab, NewTab);
}

void UMenuSettingSystemWidget::OnManagerSettingsApplied()
{
    // Update button states after apply
    if (UIManager && StateManager)
    {
        UIManager->UpdateActionButtonStates(StateManager->HasPendingChanges(), StateManager->IsLoading());
    }

    // Broadcast to Blueprint
    OnSettingsAppliedDelegate.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: Settings applied"));
}

void UMenuSettingSystemWidget::OnManagerSettingsReset()
{
    // Update button states after reset
    if (UIManager && StateManager)
    {
        UIManager->UpdateActionButtonStates(StateManager->HasPendingChanges(), StateManager->IsLoading());
    }

    // Broadcast to Blueprint
    OnSettingsResetDelegate.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: Settings reset"));
}

void UMenuSettingSystemWidget::OnManagerFadeOutCompleted()
{
    // Broadcast menu closed event
    OnMenuClosedDelegate.Broadcast();

    // Remove widget from parent
    this->RemoveFromParent();

    UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: Fade out completed, widget removed"));
}

// === DEBUG FUNCTIONS ===

void UMenuSettingSystemWidget::DebugPrintManagerStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("=== MenuSettingSystemWidget Manager Status ==="));
    UE_LOG(LogTemp, Log, TEXT("TabManager: %s"), TabManager ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("StateManager: %s"), StateManager ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("AudioManager: %s"), AudioManager ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("WidgetFactory: %s"), WidgetFactory ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("UIManager: %s"), UIManager ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("InputHandler: %s"), InputHandler ? TEXT("Valid") : TEXT("NULL"));

    if (StateManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Has Pending Changes: %s"), StateManager->HasPendingChanges() ? TEXT("true") : TEXT("false"));
        UE_LOG(LogTemp, Log, TEXT("Is Loading: %s"), StateManager->IsLoading() ? TEXT("true") : TEXT("false"));
    }

    if (TabManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Current Tab: %d"), TabManager->GetCurrentTab());
        UE_LOG(LogTemp, Log, TEXT("Total Tabs: %d"), TabManager->GetTabCount());
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Manager Status ==="));
}

void UMenuSettingSystemWidget::DebugWidgetHierarchy()
{
    UE_LOG(LogTemp, Warning, TEXT("=== WIDGET HIERARCHY DEBUG ==="));

    // Check main widget (this)
    UE_LOG(LogTemp, Warning, TEXT("MenuSettingSystemWidget:"));
    UE_LOG(LogTemp, Warning, TEXT("  Visibility: %s"), GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("Not Visible"));
    UE_LOG(LogTemp, Warning, TEXT("  IsVisible: %s"), IsVisible() ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("  DesiredSize: %s"), *GetDesiredSize().ToString());

    // Check ContentSwitcher
    if (ContentSwitcher)
    {
        UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher:"));
        UE_LOG(LogTemp, Warning, TEXT("  Visibility: %s"), ContentSwitcher->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("Not Visible"));
        UE_LOG(LogTemp, Warning, TEXT("  IsVisible: %s"), ContentSwitcher->IsVisible() ? TEXT("true") : TEXT("false"));
        UE_LOG(LogTemp, Warning, TEXT("  DesiredSize: %s"), *ContentSwitcher->GetDesiredSize().ToString());
        UE_LOG(LogTemp, Warning, TEXT("  Children Count: %d"), ContentSwitcher->GetChildrenCount());
        UE_LOG(LogTemp, Warning, TEXT("  Active Index: %d"), ContentSwitcher->GetActiveWidgetIndex());

        // Check all children
        for (int32 i = 0; i < ContentSwitcher->GetChildrenCount(); ++i)
        {
            UWidget* Child = ContentSwitcher->GetChildAt(i);
            if (Child)
            {
                UE_LOG(LogTemp, Warning, TEXT("  Child[%d] (%s):"), i, *Child->GetClass()->GetName());
                UE_LOG(LogTemp, Warning, TEXT("    Visibility: %s"), Child->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("Not Visible"));
                UE_LOG(LogTemp, Warning, TEXT("    IsVisible: %s"), Child->IsVisible() ? TEXT("true") : TEXT("false"));
                UE_LOG(LogTemp, Warning, TEXT("    RenderOpacity: %f"), Child->GetRenderOpacity());
                UE_LOG(LogTemp, Warning, TEXT("    DesiredSize: %s"), *Child->GetDesiredSize().ToString());
                UE_LOG(LogTemp, Warning, TEXT("    IsInViewport: %s"), Child->IsInViewport() ? TEXT("true") : TEXT("false"));

                // Check if child has any children (for debugging complex widgets)
                if (UPanelWidget* PanelChild = Cast<UPanelWidget>(Child))
                {
                    UE_LOG(LogTemp, Warning, TEXT("    Has %d sub-children"), PanelChild->GetChildrenCount());
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("  Child[%d]: NULL"), i);
            }
        }

        // Check active widget specifically
        UWidget* ActiveWidget = ContentSwitcher->GetActiveWidget();
        if (ActiveWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Active Widget (%s):"), *ActiveWidget->GetClass()->GetName());
            UE_LOG(LogTemp, Warning, TEXT("  Should be visible and rendered"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Active Widget is NULL!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ContentSwitcher is NULL!"));
    }

    // Check parent hierarchy
    UWidget* Parent = GetParent();
    int32 HierarchyLevel = 1;
    while (Parent && HierarchyLevel < 10) // Prevent infinite loop
    {
        UE_LOG(LogTemp, Warning, TEXT("Parent[%d] (%s):"), HierarchyLevel, *Parent->GetClass()->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  Visibility: %s"), Parent->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("Not Visible"));
        UE_LOG(LogTemp, Warning, TEXT("  IsVisible: %s"), Parent->IsVisible() ? TEXT("true") : TEXT("false"));

        Parent = Parent->GetParent();
        HierarchyLevel++;
    }

    UE_LOG(LogTemp, Warning, TEXT("=== END WIDGET HIERARCHY DEBUG ==="));
}

void UMenuSettingSystemWidget::ForceRefreshAllTabs()
{
    if (WidgetFactory)
    {
        WidgetFactory->RefreshAllWidgets();
        UE_LOG(LogTemp, Log, TEXT("MenuSettingSystemWidget: All tabs refreshed"));
    }
}