#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "PlayerProfileHUDWidget.generated.h"

class UImage;
class UPlayerIDManager;
class UIconSlotWidget;
class UIconSlotContentWidget;

UCLASS()
class HORRORGAME_API UPlayerProfileHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // UI Components - bind these in the Widget Blueprint
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerIDText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SteamIDText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CreatedDateText;

    UPROPERTY(meta = (BindWidget))
    UBorder* HUDBorder;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    UPROPERTY(meta = (BindWidget))
    UButton* RefreshButton;

    UPROPERTY(meta = (BindWidgetOptional))
    UIconSlotWidget* IconSlotWidget; 

    // Class để tạo content widget:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TSubclassOf<UIconSlotContentWidget> IconSlotContentWidgetClass;

    UPROPERTY()
    UIconSlotContentWidget* ProfileContentInstance;

    UPROPERTY(meta = (BindWidgetOptional))
    UBorder* ProfileInfoBoxBorder;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // Button click handlers
    UFUNCTION()
    void OnCloseButtonClicked();

    UFUNCTION()
    void OnRefreshButtonClicked();

private:
    UPROPERTY()
    UPlayerIDManager* PlayerIDManager;

    FTimerHandle AnimTimer;

    void PlayShowAnimation();
    void PlayHideAnimation();
    void UpdatePlayerInfo();
    void SetupHUDAppearance();

    UFUNCTION()
    void HandleIconClicked(int32 ImageIndex, UIconSlotWidget* Source);

    // Helper
    void ShowContentInProfileBox(int32 ImageIndex);
    void HideProfileContent();
};