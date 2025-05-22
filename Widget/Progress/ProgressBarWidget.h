#pragma once
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ProgressBarWidget.generated.h"

class UImage;

UCLASS()
class HORRORGAME_API UProgressBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ProgressBar named "EscapeProgressBar" in UMG Designer
    UPROPERTY(meta = (BindWidget))
    UProgressBar* EscapeProgressBar;

    UFUNCTION(BlueprintCallable, Category = "Escape")
    void SetProgressPercent(float InPercent);

    UPROPERTY(meta = (BindWidget))
    class UImage* NextKeyImage;

    UFUNCTION(BlueprintCallable, Category = "Escape")
    void SetNextKeyImage(UTexture2D* InTexture);

    UPROPERTY()
    UImage* PhaseImage;

    UFUNCTION(BlueprintCallable, Category = "QTE")
    void SetPhaseImage(UTexture2D* InTexture);

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    class UWidgetAnimation* PressAnim;

    UFUNCTION(BlueprintCallable)
    void PlayPress();
};
