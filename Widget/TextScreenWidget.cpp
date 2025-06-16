#include "TextScreenWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UTextScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!DisplayText.IsEmpty())
    {
        SetTextBlockText(DisplayText);
        CurrentTextIndex = INDEX_NONE;
    }
    else if (TextHistory.Num() > 0)
    {
        CurrentTextIndex = 0;
        SetTextBlockText(TextHistory[0]);
    }
}

void UTextScreenWidget::SetTextBlockText(const FText& NewText)
{
    if (!TextBlock) return;

    TextBlock->SetText(NewText);
    TextBlock->SetVisibility(ESlateVisibility::Visible);

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

    SetHideAnimation();
}

void UTextScreenWidget::HideTextBlock()
{
    if (TextBlock)
    {
        TextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
}
