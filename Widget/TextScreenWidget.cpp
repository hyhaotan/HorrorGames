#include "TextScreenWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UTextScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!DisplayText.IsEmpty())
    {
        SetTextBlockText(DisplayText);
    }
}

void UTextScreenWidget::SetTextBlockText(const FText& NewText)
{
    if (!TextBlock) return;

    TextBlock->SetText(NewText);
    TextBlock->SetVisibility(ESlateVisibility::Visible);
	SetShowAnimation();

    if (GetWorld())
    {
        FTimerHandle TextHiddenTimerHandle;
        GetWorld()->GetTimerManager().ClearTimer(TextHiddenTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(
            TextHiddenTimerHandle,
            this,
            &UTextScreenWidget::HideTextBlock,
            3.0f,
            false
        );
    }
}

void UTextScreenWidget::HideTextBlock()
{
    if (TextBlock)
    {
        TextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    SetHideAnimation();
}
