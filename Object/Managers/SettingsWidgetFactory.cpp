#include "SettingsWidgetFactory.h"
#include "HorrorGame/Widget/Settings/MainSettings/MenuSettingSystemWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/GameplayWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/GraphicsWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/AudioWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/Voice/VoiceWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/ControlsWidget.h"
#include "HorrorGame/Widget/Settings/SettingsTab/AccessibilityWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"

USettingsWidgetFactory::USettingsWidgetFactory()
    : GameplayWidget(nullptr)
    , GraphicsWidget(nullptr)
    , AudioWidget(nullptr)
    , VoiceWidget(nullptr)
    , ControlsWidget(nullptr)
    , AccessibilityWidget(nullptr)
    , ParentWidget(nullptr)
    , ContentSwitcher(nullptr)
{
}


void USettingsWidgetFactory::Initialize(UMenuSettingSystemWidget* InParentWidget)
{
    if (!InParentWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsWidgetFactory::Initialize - ParentWidget is null!"));
        return;
    }

    ParentWidget = InParentWidget;
    ContentSwitcher = InParentWidget->GetContentSwitcher();

    if (!ContentSwitcher.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsWidgetFactory::Initialize - ContentSwitcher not found!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("SettingsWidgetFactory::Initialize - ContentSwitcher found, proceeding with initialization"));

    // DON'T create widgets here - let CreateAllChildWidgets be called explicitly
    UE_LOG(LogTemp, Log, TEXT("SettingsWidgetFactory initialized - widgets will be created on demand"));
}

void USettingsWidgetFactory::CreateAllChildWidgets()
{
    if (!ContentSwitcher.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsWidgetFactory::CreateAllChildWidgets - ContentSwitcher is invalid!"));
        return;
    }

    if (!ParentWidget.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsWidgetFactory::CreateAllChildWidgets - ParentWidget is invalid!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("=== STARTING WIDGET CREATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher current children: %d"), ContentSwitcher->GetChildrenCount());

    // Clear existing children if any
    if (ContentSwitcher->GetChildrenCount() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Clearing existing ContentSwitcher children"));
        ContentSwitcher->ClearChildren();
    }

    AllChildWidgets.Empty();

    // Ensure ContentSwitcher is visible and ready
    ContentSwitcher->SetVisibility(ESlateVisibility::Visible);
    ContentSwitcher->SetRenderOpacity(1.0f);

    UE_LOG(LogTemp, Warning, TEXT("Creating widgets in order..."));

    // Create each widget in order - with improved error handling and detailed logging
    bool bAllCreated = true;
    int32 CreatedCount = 0;

    if (CreateGameplayWidget()) CreatedCount++; else bAllCreated = false;
    if (CreateGraphicsWidget()) CreatedCount++; else bAllCreated = false;
    if (CreateAudioWidget()) CreatedCount++; else bAllCreated = false;
    if (CreateVoiceWidget()) CreatedCount++; else bAllCreated = false;
    if (CreateControlsWidget()) CreatedCount++; else bAllCreated = false;
    if (CreateAccessibilityWidget()) CreatedCount++; else bAllCreated = false;

    UE_LOG(LogTemp, Warning, TEXT("Widget creation summary: %d/6 widgets created successfully"), CreatedCount);

    // Cache all widgets for easy access
    CacheAllWidgets();

    if (bAllCreated && CreatedCount == 6)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== ALL WIDGETS CREATED SUCCESSFULLY ==="));

        // CRITICAL: Initialize widget visibility states
        for (int32 i = 0; i < AllChildWidgets.Num(); ++i)
        {
            if (AllChildWidgets[i])
            {
                if (i == 0)
                {
                    // First widget should be visible
                    AllChildWidgets[i]->SetVisibility(ESlateVisibility::Visible);
                    AllChildWidgets[i]->SetRenderOpacity(1.0f);
                    UE_LOG(LogTemp, Warning, TEXT("Set widget[%d] (%s) as VISIBLE"), i, *AllChildWidgets[i]->GetClass()->GetName());
                }
                else
                {
                    // Others should be collapsed
                    AllChildWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
                    AllChildWidgets[i]->SetRenderOpacity(0.0f);
                    UE_LOG(LogTemp, Warning, TEXT("Set widget[%d] (%s) as COLLAPSED"), i, *AllChildWidgets[i]->GetClass()->GetName());
                }

                // Force layout update for each widget
                AllChildWidgets[i]->InvalidateLayoutAndVolatility();
            }
        }

        // Set ContentSwitcher active widget to first child
        if (ContentSwitcher->GetChildrenCount() > 0)
        {
            ContentSwitcher->SetActiveWidgetIndex(0);
            UWidget* FirstWidget = ContentSwitcher->GetActiveWidget();
            if (FirstWidget)
            {
                FirstWidget->SetVisibility(ESlateVisibility::Visible);
                FirstWidget->SetRenderOpacity(1.0f);
                UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher active widget set to: %s"), *FirstWidget->GetClass()->GetName());
            }
        }

        // Force ContentSwitcher layout update
        ContentSwitcher->InvalidateLayoutAndVolatility();
        ContentSwitcher->ForceLayoutPrepass();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("=== WIDGET CREATION FAILED ==="));
        UE_LOG(LogTemp, Error, TEXT("Some child widgets failed to create! Created: %d/6"), CreatedCount);
    }

    // Final debug output
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL STATE ==="));
    UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher children count: %d"), ContentSwitcher->GetChildrenCount());
    UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher visibility: %s"),
        ContentSwitcher->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("NotVisible"));
    UE_LOG(LogTemp, Warning, TEXT("ContentSwitcher active index: %d"), ContentSwitcher->GetActiveWidgetIndex());

    if (UWidget* ActiveWidget = ContentSwitcher->GetActiveWidget())
    {
        UE_LOG(LogTemp, Warning, TEXT("Active widget: %s"), *ActiveWidget->GetClass()->GetName());
        UE_LOG(LogTemp, Warning, TEXT("Active widget visibility: %s"),
            ActiveWidget->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("NotVisible"));
    }

    UE_LOG(LogTemp, Warning, TEXT("=== END WIDGET CREATION ==="));
}

bool USettingsWidgetFactory::CreateGameplayWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating GameplayWidget..."));

    if (GameplayWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayWidget already exists, skipping creation"));
        return true;
    }

    if (!ParentWidget.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ParentWidget is invalid, cannot create GameplayWidget"));
        return false;
    }

    // Check if class exists
    UClass* GameplayClass = UGameplayWidget::StaticClass();
    if (!GameplayClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UGameplayWidget::StaticClass() returned NULL!"));
        UE_LOG(LogTemp, Error, TEXT("Check if GameplayWidget.h is included and class is properly defined"));
        return CreateSimpleFallbackWidget(TEXT("Gameplay Settings - Class Missing"));
    }

    UE_LOG(LogTemp, Warning, TEXT("GameplayWidget class found: %s"), *GameplayClass->GetName());

    // Try to create widget
    GameplayWidget = CreateWidget<UGameplayWidget>(ParentWidget.Get(), GameplayClass);

    if (GameplayWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayWidget created successfully"));
        AddWidgetToSwitcher(GameplayWidget);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create GameplayWidget even with valid class"));
        return CreateSimpleFallbackWidget(TEXT("Gameplay Settings - Creation Failed"));
    }
}

bool USettingsWidgetFactory::CreateGraphicsWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating GraphicsWidget..."));

    if (GraphicsWidget)
    {
        return true;
    }

    if (!ParentWidget.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ParentWidget is invalid, cannot create GraphicsWidget"));
        return false;
    }

    UClass* GraphicsClass = UGraphicsWidget::StaticClass();
    if (!GraphicsClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UGraphicsWidget::StaticClass() returned NULL!"));
        return CreateSimpleFallbackWidget(TEXT("Graphics Settings - Class Missing"));
    }

    GraphicsWidget = CreateWidget<UGraphicsWidget>(ParentWidget.Get(), GraphicsClass);

    if (GraphicsWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphicsWidget created successfully"));
        AddWidgetToSwitcher(GraphicsWidget);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create GraphicsWidget"));
        return CreateSimpleFallbackWidget(TEXT("Graphics Settings - Creation Failed"));
    }
}

bool USettingsWidgetFactory::CreateAudioWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating AudioWidget..."));

    if (AudioWidget)
    {
        return true;
    }

    if (!ParentWidget.IsValid())
    {
        return false;
    }

    UClass* AudioClass = UAudioWidget::StaticClass();
    if (!AudioClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UAudioWidget::StaticClass() returned NULL!"));
        return CreateSimpleFallbackWidget(TEXT("Audio Settings - Class Missing"));
    }

    AudioWidget = CreateWidget<UAudioWidget>(ParentWidget.Get(), AudioClass);

    if (AudioWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioWidget created successfully"));
        AddWidgetToSwitcher(AudioWidget);
        return true;
    }
    else
    {
        return CreateSimpleFallbackWidget(TEXT("Audio Settings - Creation Failed"));
    }
}

bool USettingsWidgetFactory::CreateVoiceWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating VoiceWidget..."));

    if (VoiceWidget)
    {
        return true;
    }

    if (!ParentWidget.IsValid())
    {
        return false;
    }

    UClass* VoiceClass = UVoiceWidget::StaticClass();
    if (!VoiceClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UVoiceWidget::StaticClass() returned NULL!"));
        return CreateSimpleFallbackWidget(TEXT("Voice Settings - Class Missing"));
    }

    VoiceWidget = CreateWidget<UVoiceWidget>(ParentWidget.Get(), VoiceClass);

    if (VoiceWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("VoiceWidget created successfully"));
        AddWidgetToSwitcher(VoiceWidget);
        return true;
    }
    else
    {
        return CreateSimpleFallbackWidget(TEXT("Voice Settings - Creation Failed"));
    }
}

bool USettingsWidgetFactory::CreateControlsWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating ControlsWidget..."));

    if (ControlsWidget)
    {
        return true;
    }

    if (!ParentWidget.IsValid())
    {
        return false;
    }

    UClass* ControlsClass = UControlsWidget::StaticClass();
    if (!ControlsClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UControlsWidget::StaticClass() returned NULL!"));
        return CreateSimpleFallbackWidget(TEXT("Controls Settings - Class Missing"));
    }

    ControlsWidget = CreateWidget<UControlsWidget>(ParentWidget.Get(), ControlsClass);

    if (ControlsWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("ControlsWidget created successfully"));
        AddWidgetToSwitcher(ControlsWidget);
        return true;
    }
    else
    {
        return CreateSimpleFallbackWidget(TEXT("Controls Settings - Creation Failed"));
    }
}

bool USettingsWidgetFactory::CreateAccessibilityWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating AccessibilityWidget..."));

    if (AccessibilityWidget)
    {
        return true;
    }

    if (!ParentWidget.IsValid())
    {
        return false;
    }

    UClass* AccessibilityClass = UAccessibilityWidget::StaticClass();
    if (!AccessibilityClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UAccessibilityWidget::StaticClass() returned NULL!"));
        return CreateSimpleFallbackWidget(TEXT("Accessibility Settings - Class Missing"));
    }

    AccessibilityWidget = CreateWidget<UAccessibilityWidget>(ParentWidget.Get(), AccessibilityClass);

    if (AccessibilityWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("AccessibilityWidget created successfully"));
        AddWidgetToSwitcher(AccessibilityWidget);
        return true;
    }
    else
    {
        return CreateSimpleFallbackWidget(TEXT("Accessibility Settings - Creation Failed"));
    }
}

bool USettingsWidgetFactory::CreateSimpleFallbackWidget(const FString& LabelText)
{
    if (!ParentWidget.IsValid() || !ContentSwitcher.IsValid())
    {
        return false;
    }

    // Create a simple UserWidget as fallback container
    UUserWidget* FallbackWidget = CreateWidget<UUserWidget>(ParentWidget.Get(), UUserWidget::StaticClass());

    if (FallbackWidget)
    {
        // Create a TextBlock using the widget's WidgetTree
        UTextBlock* FallbackTextWidget = FallbackWidget->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());

        if (FallbackTextWidget)
        {
            FallbackTextWidget->SetText(FText::FromString(LabelText));
            FallbackTextWidget->SetJustification(ETextJustify::Center);

            // Set some basic styling
            FSlateFontInfo FontInfo;
            FontInfo.Size = 16;
            FallbackTextWidget->SetFont(FontInfo);
            FallbackTextWidget->SetColorAndOpacity(FLinearColor::White);

            // Set the TextBlock as the root widget of our UserWidget
            FallbackWidget->WidgetTree->RootWidget = FallbackTextWidget;
        }

        AddWidgetToSwitcher(FallbackWidget);

        UE_LOG(LogTemp, Warning, TEXT("Created fallback UserWidget with TextBlock: %s"), *LabelText);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create fallback UserWidget!"));
        return false;
    }
}

UUserWidget* USettingsWidgetFactory::CreateFallbackWidget(const FString& LabelText)
{
    // DEPRECATED - DO NOT USE
    UE_LOG(LogTemp, Error, TEXT("CreateFallbackWidget is deprecated - use CreateSimpleFallbackWidget instead"));
    return nullptr;
}

void USettingsWidgetFactory::ValidateWidgetCreation()
{
    int32 ValidWidgets = 0;
    int32 TotalWidgets = 6;

    if (GameplayWidget) ValidWidgets++;
    if (GraphicsWidget) ValidWidgets++;
    if (AudioWidget) ValidWidgets++;
    if (VoiceWidget) ValidWidgets++;
    if (ControlsWidget) ValidWidgets++;
    if (AccessibilityWidget) ValidWidgets++;

    UE_LOG(LogTemp, Log, TEXT("Widget Creation Summary: %d/%d widgets created successfully"), ValidWidgets, TotalWidgets);

    if (ContentSwitcher.IsValid())
    {
        int32 SwitcherChildCount = ContentSwitcher->GetChildrenCount();
        UE_LOG(LogTemp, Log, TEXT("ContentSwitcher has %d children"), SwitcherChildCount);

        if (SwitcherChildCount != ValidWidgets)
        {
            UE_LOG(LogTemp, Error, TEXT("Mismatch: Created %d widgets but ContentSwitcher has %d children"), ValidWidgets, SwitcherChildCount);
        }
    }
}

void USettingsWidgetFactory::RefreshAllWidgets()
{
    for (UWidget* Widget : AllChildWidgets)
    {
        RefreshSpecificWidget(Widget);
    }
    UE_LOG(LogTemp, Log, TEXT("SettingsWidgetFactory: All widgets refreshed"));
}

void USettingsWidgetFactory::RefreshCurrentWidget(int32 CurrentTabIndex)
{
    if (IsWidgetIndexValid(CurrentTabIndex))
    {
        RefreshSpecificWidget(AllChildWidgets[CurrentTabIndex]);
        UE_LOG(LogTemp, Log, TEXT("SettingsWidgetFactory: Current widget (index %d) refreshed"), CurrentTabIndex);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsWidgetFactory::RefreshCurrentWidget - Invalid tab index: %d"), CurrentTabIndex);
    }
}

void USettingsWidgetFactory::RefreshWidgetByIndex(int32 WidgetIndex)
{
    if (IsWidgetIndexValid(WidgetIndex))
    {
        RefreshSpecificWidget(AllChildWidgets[WidgetIndex]);
        UE_LOG(LogTemp, Log, TEXT("SettingsWidgetFactory: Widget at index %d refreshed"), WidgetIndex);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsWidgetFactory::RefreshWidgetByIndex - Invalid widget index: %d"), WidgetIndex);
    }
}

UWidget* USettingsWidgetFactory::GetWidgetByIndex(int32 Index) const
{
    if (IsWidgetIndexValid(Index))
    {
        return AllChildWidgets[Index];
    }
    return nullptr;
}

bool USettingsWidgetFactory::ValidateAllWidgets() const
{
    bool bAllValid = true;

    for (int32 i = 0; i < AllChildWidgets.Num(); ++i)
    {
        if (!IsWidgetValid(i))
        {
            bAllValid = false;
            UE_LOG(LogTemp, Warning, TEXT("SettingsWidgetFactory::ValidateAllWidgets - Widget at index %d is invalid"), i);
        }
    }

    return bAllValid;
}

bool USettingsWidgetFactory::IsWidgetValid(int32 WidgetIndex) const
{
    return IsWidgetIndexValid(WidgetIndex) && IsValid(AllChildWidgets[WidgetIndex]);
}

void USettingsWidgetFactory::AddWidgetToSwitcher(UWidget* Widget)
{
    if (Widget && ContentSwitcher.IsValid())
    {
        // Set widget properties before adding to switcher
        Widget->SetVisibility(ESlateVisibility::Collapsed); // Start collapsed
        Widget->SetRenderOpacity(1.0f);

        // Add to switcher
        ContentSwitcher->AddChild(Widget);

        int32 NewIndex = ContentSwitcher->GetChildrenCount() - 1;

        UE_LOG(LogTemp, Warning, TEXT("Widget %s added to switcher at index %d. Total children: %d"),
            *Widget->GetClass()->GetName(), NewIndex, ContentSwitcher->GetChildrenCount());

        // Force layout update
        Widget->InvalidateLayoutAndVolatility();

        // Debug the widget after adding
        UE_LOG(LogTemp, Warning, TEXT("Added widget state - Visibility: %s, Opacity: %f, DesiredSize: %s"),
            Widget->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("NotVisible"),
            Widget->GetRenderOpacity(),
            *Widget->GetDesiredSize().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to add widget to switcher - Widget: %s, ContentSwitcher: %s"),
            Widget ? *Widget->GetClass()->GetName() : TEXT("NULL"),
            ContentSwitcher.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
    }
}

void USettingsWidgetFactory::CacheAllWidgets()
{
    AllChildWidgets.Empty();
    AllChildWidgets.Add(GameplayWidget);
    AllChildWidgets.Add(GraphicsWidget);
    AllChildWidgets.Add(AudioWidget);
    AllChildWidgets.Add(VoiceWidget);
    AllChildWidgets.Add(ControlsWidget);
    AllChildWidgets.Add(AccessibilityWidget);

    UE_LOG(LogTemp, Log, TEXT("Cached %d widgets in AllChildWidgets array"), AllChildWidgets.Num());
}

bool USettingsWidgetFactory::IsWidgetIndexValid(int32 Index) const
{
    return Index >= 0 && Index < AllChildWidgets.Num();
}

void USettingsWidgetFactory::RefreshSpecificWidget(UWidget* Widget)
{
    if (!Widget)
        return;

    // Try to call RefreshValues function on each widget type
    if (UGameplayWidget* GameplayPtr = Cast<UGameplayWidget>(Widget))
    {
        CallWidgetRefreshFunction(GameplayPtr, TEXT("RefreshValues"));
    }
    else if (UGraphicsWidget* GraphicsPtr = Cast<UGraphicsWidget>(Widget))
    {
        CallWidgetRefreshFunction(GraphicsPtr, TEXT("RefreshValues"));
    }
    else if (UAudioWidget* AudioPtr = Cast<UAudioWidget>(Widget))
    {
        CallWidgetRefreshFunction(AudioPtr, TEXT("RefreshValues"));
    }
    else if (UVoiceWidget* VoicePtr = Cast<UVoiceWidget>(Widget))
    {
        CallWidgetRefreshFunction(VoicePtr, TEXT("RefreshValues"));
    }
    else if (UControlsWidget* ControlsPtr = Cast<UControlsWidget>(Widget))
    {
        CallWidgetRefreshFunction(ControlsPtr, TEXT("RefreshValues"));
    }
    else if (UAccessibilityWidget* AccessibilityPtr = Cast<UAccessibilityWidget>(Widget))
    {
        CallWidgetRefreshFunction(AccessibilityPtr, TEXT("RefreshValues"));
    }
}

bool USettingsWidgetFactory::CallWidgetRefreshFunction(UWidget* Widget, const FString& FunctionName)
{
    if (!Widget)
        return false;

    FName FuncFName(*FunctionName);

    UFunction* RefreshFunction = Widget->GetClass()->FindFunctionByName(FuncFName);
    if (RefreshFunction)
    {
        Widget->ProcessEvent(RefreshFunction, nullptr);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsWidgetFactory::CallWidgetRefreshFunction - Function %s not found on widget %s"),
            *FunctionName, *Widget->GetClass()->GetName());
        return false;
    }
}

// === DEBUG FUNCTIONS ===

void USettingsWidgetFactory::DebugPrintWidgetInfo() const
{
    UE_LOG(LogTemp, Log, TEXT("=== SettingsWidgetFactory Debug Info ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Child Widgets: %d"), AllChildWidgets.Num());
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: %s"), GameplayWidget ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("GraphicsWidget: %s"), GraphicsWidget ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("AudioWidget: %s"), AudioWidget ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("VoiceWidget: %s"), VoiceWidget ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("ControlsWidget: %s"), ControlsWidget ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("AccessibilityWidget: %s"), AccessibilityWidget ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("ContentSwitcher: %s"), ContentSwitcher.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
    UE_LOG(LogTemp, Log, TEXT("ParentWidget: %s"), ParentWidget.IsValid() ? TEXT("Valid") : TEXT("Invalid"));

    if (ContentSwitcher.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("ContentSwitcher Children Count: %d"), ContentSwitcher->GetChildrenCount());
        UE_LOG(LogTemp, Log, TEXT("ContentSwitcher Active Widget Index: %d"), ContentSwitcher->GetActiveWidgetIndex());

        UWidget* ActiveWidget = ContentSwitcher->GetActiveWidget();
        UE_LOG(LogTemp, Log, TEXT("Active Widget: %s"), ActiveWidget ? *ActiveWidget->GetClass()->GetName() : TEXT("NULL"));
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Debug Info ==="));
}

void USettingsWidgetFactory::DebugValidateAllWidgets() const
{
    UE_LOG(LogTemp, Log, TEXT("SettingsWidgetFactory: Validating all widgets..."));

    bool bAllValid = ValidateAllWidgets();
    if (bAllValid)
    {
        UE_LOG(LogTemp, Log, TEXT("All widgets are valid"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Some widgets are invalid - check logs above"));
    }
}