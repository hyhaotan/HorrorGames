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

void UKeyBindingWidget::OnKeySelected(FKey SelectedKey, bool bIsGamepadKey)
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

    // Cập nhật key binding tương ứng
    if (bIsGamepadKey)
    {
        CurrentKeyBinding.CurrentGamepadKey = SelectedKey;
    }
    else
    {
        CurrentKeyBinding.CurrentKey = SelectedKey;
    }

    // Cập nhật UI text
    if (KeyText)
    {
        KeyText->SetText(FText::FromString(SelectedKey.GetDisplayName().ToString()));
    }

    // Gọi delegate với 3 tham số (ActionTag, Key, bIsGamepadKey)
    OnKeyBindingChanged.ExecuteIfBound(CurrentKeyBinding.ActionTag, SelectedKey, bIsGamepadKey);
}

FReply UKeyBindingWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (bIsWaitingForKey)
    {
        FKey PressedKey = InKeyEvent.GetKey();
        bool bIsGamepadKey = PressedKey.IsGamepadKey();
        OnKeySelected(PressedKey, bIsGamepadKey);
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UKeyBindingWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsWaitingForKey)
    {
        OnKeySelected(InMouseEvent.GetEffectingButton(), false);
        return FReply::Handled();
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}