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
	const FText DefaultConfirmText = FText::FromString(TEXT("Confirm"));
	const FText DefaultCancelText = FText::FromString(TEXT("Cancel"));
	const FText ConfirmTextToUse = ConfirmText.IsEmpty() ? DefaultConfirmText : ConfirmText;
	const FText CancelTextToUse = CancelText.IsEmpty() ? DefaultCancelText : CancelText;

    if (TitleText)
        TitleText->SetText(Title);

    if (MessageText)
        MessageText->SetText(Message);

    if (ConfirmButtonText)
        ConfirmButtonText->SetText(ConfirmTextToUse);

    if (CancelButtonText)
        CancelButtonText->SetText(CancelTextToUse);

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