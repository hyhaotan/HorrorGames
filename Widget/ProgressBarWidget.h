#pragma once
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ProgressBarWidget.generated.h"

UCLASS()
class HORRORGAME_API UProgressBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ProgressBar named "EscapeProgressBar" in UMG Designer
    UPROPERTY(meta = (BindWidget))
    UProgressBar* EscapeProgressBar;

    UFUNCTION(BlueprintCallable, Category = "Escape")
    void SetProgressPercent(float InPercent)
    {
        if (EscapeProgressBar)
        {
            EscapeProgressBar->SetPercent(InPercent);
        }
    }

    // TextBlock for next QTE key
    UPROPERTY(meta = (BindWidget))
    UTextBlock* NextKeyText;

    UFUNCTION(BlueprintCallable, Category = "Escape")
    void SetNextKey(const FString& KeyName)
    {
        if (NextKeyText)
        {
            NextKeyText->SetText(FText::FromString(KeyName));
        }
    }

    // TextBlock to display current phase
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PhaseText;

    /**
     * Set the displayed QTE phase label (e.g. "WASD", "Arrows", "Opposite").
     */
    UFUNCTION(BlueprintCallable, Category = "QTE")
    void SetPhaseText(const FString& PhaseName)
    {
        if (PhaseText)
        {
            PhaseText->SetText(FText::FromString(PhaseName));
        }
    }
};
