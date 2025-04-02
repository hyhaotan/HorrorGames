#include "HorrorGame/Widget/Settings/SelectionWidget.h"
#include "CommonTextBlock.h"
#include "Logging/StructuredLog.h"

USelectionWidget::USelectionWidget()
{
    CurrentSelection = 0;
    SetIsFocusable(true);
    SetVisibilityInternal(ESlateVisibility::Visible);
}

void USelectionWidget::NativeConstruct()
{
    if (Options.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No options set for selection widget"));
        return;
    }

    UpdateCurrentSelection();

    FCustomWidgetNavigationDelegate NavigationDelegate;
    NavigationDelegate.BindDynamic(this, &USelectionWidget::OnNavigation);
    SetNavigationRuleCustom(EUINavigation::Left, NavigationDelegate);
    SetNavigationRuleCustom(EUINavigation::Right, NavigationDelegate);
}

void USelectionWidget::Clear()
{
    Options.Reset();
}

void USelectionWidget::AddOption(const FSelectionOption& InOption)
{
    Options.Add(InOption);
    UpdateCurrentSelection();
}

void USelectionWidget::SetCurrentSelection(int InIndex)
{
	check(InIndex >= 0 && InIndex < Options.Num());
    CurrentSelection = InIndex;
    UpdateCurrentSelection();
}

void USelectionWidget::SelectPrevious()
{
    OnNavigation(EUINavigation::Left);
}

void USelectionWidget::SelectNext()
{
    OnNavigation(EUINavigation::Right);
}

UWidget* USelectionWidget::OnNavigation(EUINavigation InNavigation)
{
    check(InNavigation == EUINavigation::Left || InNavigation == EUINavigation::Right);

    const auto Direction = InNavigation == EUINavigation::Left ? -1 : 1;

    CurrentSelection += Direction;
    if (CurrentSelection < 0)
    {
        CurrentSelection = Options.Num() - 1;
    }
    else if (CurrentSelection >= Options.Num())
    {
        CurrentSelection = 0;
    }

    UpdateCurrentSelection();

    OnSelectionChange.ExecuteIfBound(CurrentSelection);

    return this;
}

void USelectionWidget::UpdateCurrentSelection()
{
    Label->SetText(Options[CurrentSelection].Label);
}
