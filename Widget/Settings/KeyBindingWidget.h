#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "ControlsWidget.h"
#include "KeyBindingWidget.generated.h"

DECLARE_DELEGATE_TwoParams(FOnKeyBindingChanged, FName, FKey);

UCLASS()
class HORRORGAME_API UKeyBindingWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable)
    void SetKeyBinding(const FKeyBinding& InKeyBinding);

    FOnKeyBindingChanged OnKeyBindingChanged;

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ActionNameText;

    UPROPERTY(meta = (BindWidget))
    class UButton* KeyButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* KeyText;

private:
    UFUNCTION()
    void OnKeyButtonClicked();

    UFUNCTION()
    void OnKeySelected(FKey SelectedKey);

    FKeyBinding CurrentKeyBinding;
    bool bIsWaitingForKey = false;

    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};