#include "KeyBindingWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UKeyBindingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (KeyButton)
    {
        KeyButton->OnClicked.AddDynamic(this, &UKeyBindingWidget::OnKeyButtonClicked);
    }
}

void UKeyBindingWidget::SetKeyBinding(const FKeyBinding& InKeyBinding)
{
    CurrentKeyBinding = InKeyBinding;

    if (ActionNameText)
    {
        ActionNameText->SetText(CurrentKeyBinding.ActionName);
    }

    if (KeyText)
    {
        KeyText->SetText(FText::FromString(CurrentKeyBinding.CurrentKey.GetDisplayName().ToString()));
    }
}

void UKeyBindingWidget::OnKeyButtonClicked()
{
    if (bIsWaitingForKey)
        return;

    bIsWaitingForKey = true;

    if (KeyText)
    {
        KeyText->SetText(FText::FromString("Press any key..."));
    }

    // Set focus to capture key input
    SetKeyboardFocus();
}

void UKeyBindingWidget::OnKeySelected(FKey SelectedKey)
{
    if (!bIsWaitingForKey)
        return;

    bIsWaitingForKey = false;

    // Don't allow binding to escape key
    if (SelectedKey == EKeys::Escape)
    {
        if (KeyText)
        {
            KeyText->SetText(FText::FromString(CurrentKeyBinding.CurrentKey.GetDisplayName().ToString()));
        }
        return;
    }

    CurrentKeyBinding.CurrentKey = SelectedKey;

    if (KeyText)
    {
        KeyText->SetText(FText::FromString(SelectedKey.GetDisplayName().ToString()));
    }

    OnKeyBindingChanged.ExecuteIfBound(CurrentKeyBinding.ActionTag, SelectedKey);
}

FReply UKeyBindingWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (bIsWaitingForKey)
    {
        OnKeySelected(InKeyEvent.GetKey());
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UKeyBindingWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsWaitingForKey)
    {
        OnKeySelected(InMouseEvent.GetEffectingButton());
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}