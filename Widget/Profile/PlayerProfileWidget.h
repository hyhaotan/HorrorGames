#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "PlayerProfileWidget.generated.h"

// Forward declaration
class UPlayerProfileHUDWidget;

UCLASS()
class HORRORGAME_API UPlayerProfileWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DisplayNamePlayer;

    UPROPERTY(meta = (BindWidget))
    UBorder* BorderImage;

    UPROPERTY(meta = (BindWidget))
    UImage* IconPlayer;

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* PlayerProfileBox;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerHUD")
    TSubclassOf<UPlayerProfileHUDWidget> PlayerHUDClass;

    // Hover effect colors
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hover Effects")
    FLinearColor NormalBorderColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hover Effects")
    FLinearColor HoverBorderColor = FLinearColor(0.0f, 0.7f, 1.0f, 1.0f); // Cyan glow

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hover Effects")
    float HoverAnimationDuration = 0.2f;

private:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

    // Reference to PlayerIDManager
    UPROPERTY()
    UPlayerIDManager* PlayerIDManager;

    // Current HUD widget instance
    UPROPERTY()
    UPlayerProfileHUDWidget* CurrentHUDWidget;

    FTimerHandle TimerHandle;

    // Helper functions
    void UpdatePlayerName();
    void ShowPlayerHUD();
    void HidePlayerHUD();
    void AnimateBorderToHover();
    void AnimateBorderToNormal();

    // Animation callbacks
    UFUNCTION()
    void OnBorderHoverAnimationFinished();

    UFUNCTION()
    void OnBorderNormalAnimationFinished();
};