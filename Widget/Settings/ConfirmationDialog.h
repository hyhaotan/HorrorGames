#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ConfirmationDialog.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfirmationResult, bool, bConfirmed);

UCLASS()
class HORRORGAME_API UConfirmationDialog : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Dialog")
    void ShowDialog(const FText& Title, const FText& Message, const FText& ConfirmText = FText::FromString("Confirm"), const FText& CancelText = FText::FromString("Cancel"));

    UPROPERTY(BlueprintAssignable)
    FOnConfirmationResult OnConfirmationResult;

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TitleText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* MessageText;

    UPROPERTY(meta = (BindWidget))
    class UButton* ConfirmButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* CancelButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ConfirmButtonText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CancelButtonText;

private:
    UFUNCTION()
    void OnConfirmClicked();

    UFUNCTION()
    void OnCancelClicked();
};