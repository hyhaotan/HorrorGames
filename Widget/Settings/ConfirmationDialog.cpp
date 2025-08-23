#include "ConfirmationDialog.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UConfirmationDialog::NativeConstruct()
{
    Super::NativeConstruct();

    if (ConfirmButton)
        ConfirmButton->OnClicked.AddDynamic(this, &UConfirmationDialog::OnConfirmClicked);

    if (CancelButton)
        CancelButton->OnClicked.AddDynamic(this, &UConfirmationDialog::OnCancelClicked);
}

void UConfirmationDialog::ShowDialog(const FText& Title, const FText& Message, const FText& ConfirmText, const FText& CancelText)
{
    if (TitleText)
        TitleText->SetText(Title);

    if (MessageText)
        MessageText->SetText(Message);

    if (ConfirmButtonText)
        ConfirmButtonText->SetText(ConfirmText);

    if (CancelButtonText)
        CancelButtonText->SetText(CancelText);

    AddToViewport();
    SetKeyboardFocus();
}

void UConfirmationDialog::OnConfirmClicked()
{
    OnConfirmationResult.Broadcast(true);
    RemoveFromParent();
}

void UConfirmationDialog::OnCancelClicked()
{
    OnConfirmationResult.Broadcast(false);
    RemoveFromParent();
}