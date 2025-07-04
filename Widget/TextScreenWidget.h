#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TextScreenWidget.generated.h"

class UTextBlock;

UCLASS()
class HORRORGAME_API UTextScreenWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // Called to set and show new text
    void SetTextBlockText(const FText& NewText);

    // Play show/hide animations
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void SetShowAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void SetHideAnimation();

	FText GetDisplayText() const 
    {
		return DisplayText;
    }

protected:
    // Text to display initially (designer can set in Blueprint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
    FText DisplayText;

    // TextBlock widget reference
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TextBlock;

private:
    // Hide callback
    UFUNCTION()
    void HideTextBlock();
};