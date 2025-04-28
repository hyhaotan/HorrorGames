#pragma once
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "ProgressBarWidget.generated.h"

UCLASS()
class HORRORGAME_API UProgressBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Kết nối từ UMG Designer: ProgressBar named "EscapeProgressBar"
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
};