// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TimerHandle.h"
#include "IconSlotContentWidget.generated.h"

UCLASS()
class HORRORGAME_API UIconSlotContentWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
    // UI Components
    UPROPERTY(meta = (BindWidget))
    class UBorder* ContentBorder;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TitleText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DescriptionText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* StatsText;

    UPROPERTY(meta = (BindWidget))
    class UImage* IconPreviewImage;

    // Content data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
    FText IconTitle = FText::FromString("Icon Title");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
    FText IconDescription = FText::FromString("This is the description of the selected icon. It provides detailed information about what this icon represents.");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
    FText IconStats = FText::FromString("Level: 1\nRarity: Common\nUnlocked: Yes");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor BackgroundColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.95f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor TitleColor = FLinearColor(0.0f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor DescriptionColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor StatsColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Content")
    void SetIconContent(int32 ImageIndex);

    UFUNCTION(BlueprintCallable, Category = "Content")
    void UpdateContentForImageIndex(int32 ImageIndex);

private:
    // Animation timer
    FTimerHandle AnimationTimer;

    // Setup functions
    void SetupUI();
    void PlayShowAnimation();
    void PlayHideAnimation();

    // Content data arrays for different icons
    TArray<FText> IconTitles;
    TArray<FText> IconDescriptions;
    TArray<FText> IconStatsArray;
};