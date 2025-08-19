// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Engine/TimerHandle.h"
#include "IconSlotWidget.generated.h"

class UIconSlotContentWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotHover, bool, bIsHovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIconClicked, int32, ImageIndex, UIconSlotWidget*, SlotWidget);

UCLASS()
class HORRORGAME_API UIconSlotWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // Mouse events
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
    // Widget components
    UPROPERTY(meta = (BindWidget))
    class UImage* IconImageFirst;

    UPROPERTY(meta = (BindWidget))
    class UImage* IconImageSecond;

    UPROPERTY(meta = (BindWidget))
    class UImage* IconImageThird;

    UPROPERTY(meta = (BindWidget))
    class UBorder* IconBorderFirst;

    UPROPERTY(meta = (BindWidget))
    class UBorder* IconBorderSecond;

    UPROPERTY(meta = (BindWidget))
    class UBorder* IconBorderThird;

    // Colors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor NormalBorderColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HoverBorderColor = FLinearColor(0.0f, 0.7f, 1.0f, 1.0f);

    // Content widget class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
    TSubclassOf<UIconSlotContentWidget> IconSlotContentWidgetClass;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnSlotHover OnSlotHover;

    UPROPERTY(BlueprintAssignable)
    FOnIconClicked OnIconClicked;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Icon Slot")
    void ShowContentWidget(int32 ImageIndex);

    UFUNCTION(BlueprintCallable, Category = "Icon Slot")
    void HideContentWidget();

private:
    // Timer handles
    FTimerHandle IconBorderFirstTimerHandle;
    FTimerHandle IconBorderSecondTimerHandle;
    FTimerHandle IconBorderThirdTimerHandle;

    // State
    UPROPERTY()
    bool bIsSlotHover = false;

    // Current content widget
    UPROPERTY()
    UIconSlotContentWidget* CurrentContentWidget = nullptr;

    int32 CurrentSelectedImageIndex = -1;

    void PositionContentWidget(int32 Index);

    // Helper functions
    void SetupImageHoverEvents();
    void InitializeBorderColors();
    int32 GetHoveredImageIndex(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
    int32 GetClickedImageIndex(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
    void HandleImageHover(int32 ImageIndex, bool bIsHovered);
    void HandleImageClick(int32 ImageIndex);

    // Hover events
    void OnIconImageFirstHovered();
    void OnIconImageFirstUnhovered();
    void OnIconImageSecondHovered();
    void OnIconImageSecondUnhovered();
    void OnIconImageThirdHovered();
    void OnIconImageThirdUnhovered();

    // Animation functions
    void AnimateBorderToColor(UBorder* Border, FTimerHandle& TimerHandle, const FLinearColor& TargetColor, TFunction<void()> OnFinished = nullptr, float Duration = 0.2f);
    void AnimateBorderToHover(UBorder* Border, FTimerHandle& TimerHandle, float Duration = 0.2f);
    void AnimateBorderToNormal(UBorder* Border, FTimerHandle& TimerHandle, float Duration = 0.3f);
};