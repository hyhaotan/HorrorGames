#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "MenuSettingSystemWidget.generated.h"

UCLASS()
class HORRORGAME_API UMenuSettingSystemWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
    // Tab buttons
    UPROPERTY(meta = (BindWidget))
    class UButton* GameplayTabButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* GraphicsTabButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* AudioTabButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* ControlsTabButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* AccessibilityTabButton;

    // Content panels
    UPROPERTY(meta = (BindWidget))
    class UWidgetSwitcher* ContentSwitcher;

    // Settings widgets
    UPROPERTY(meta = (BindWidget))
    class UGameplayWidget* GameplayWidget;

    UPROPERTY(meta = (BindWidget))
    class UGraphicsWidget* GraphicsWidget;

    UPROPERTY(meta = (BindWidget))
    class UAudioWidget* AudioWidget;

    UPROPERTY(meta = (BindWidget))
    class UControlsWidget* ControlsWidget;

    UPROPERTY(meta = (BindWidget))
    class UAccessibilityWidget* AccessibilityWidget;

    // Action buttons
    UPROPERTY(meta = (BindWidget))
    class UButton* ApplyButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* ResetButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* BackButton;

private:
    UFUNCTION()
    void OnGameplayTabClicked();

    UFUNCTION()
    void OnGraphicsTabClicked();

    UFUNCTION()
    void OnAudioTabClicked();

    UFUNCTION()
    void OnControlsTabClicked();

    UFUNCTION()
    void OnAccessibilityTabClicked();

    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnResetClicked();

    UFUNCTION()
    void OnBackClicked();

    void UpdateTabSelection(int32 SelectedTab);
    void SetTabButtonSelected(class UButton* Button, bool bSelected);

    int32 CurrentTab = 0;
};