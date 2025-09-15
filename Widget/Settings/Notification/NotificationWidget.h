#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Animation/WidgetAnimation.h"
#include "Engine/Texture2D.h"
#include "NotificationWidget.generated.h"

USTRUCT(BlueprintType)
struct FNotificationStyle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor BackgroundColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor TextColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor IconColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UTexture2D* IconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DisplayDuration;

    FNotificationStyle()
        : BackgroundColor(FLinearColor::Black)
        , TextColor(FLinearColor::White)
        , IconColor(FLinearColor::White)
        , IconTexture(nullptr)
        , DisplayDuration(3.0f)
    {
    }
};

UCLASS()
class HORRORGAME_API UNotificationWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UNotificationWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;

    // UI Components
    UPROPERTY(meta = (BindWidget))
    class UBorder* NotificationBorder;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* MessageText;

    UPROPERTY(meta = (BindWidget))
    class UImage* NotificationIcon;

    UPROPERTY(meta = (BindWidget))
    class UImage* BackgroundGradient;

    // Animations
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    class UWidgetAnimation* SlideInAnimation;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    class UWidgetAnimation* SlideOutAnimation;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    class UWidgetAnimation* PulseAnimation;

    // Notification Styles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification Styles")
    FNotificationStyle InfoStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification Styles")
    FNotificationStyle SuccessStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification Styles")
    FNotificationStyle WarningStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification Styles")
    FNotificationStyle ErrorStyle;

    // Sound Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* InfoSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* SuccessSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* WarningSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* ErrorSound;

private:
    // Current notification state
    UPROPERTY()
    bool bIsShowing;

    UPROPERTY()
    ENotificationType CurrentType;

    // Timer for auto-hide
    FTimerHandle HideTimerHandle;

    // Queue for multiple notifications
    UPROPERTY()
    TArray<FString> NotificationQueue;

    UPROPERTY()
    TArray<ENotificationType> TypeQueue;

    // Internal functions
    void ApplyNotificationStyle(ENotificationType Type);
    void PlayNotificationSound(ENotificationType Type);
    void ProcessNotificationQueue();

    UFUNCTION()
    void OnSlideInFinished();

    UFUNCTION()
    void OnSlideOutFinished();

public:
    // Main function to show notifications
    UFUNCTION(BlueprintCallable, Category = "Notification")
    void ShowNotification(const FString& Message, ENotificationType Type);

    UFUNCTION(BlueprintCallable, Category = "Notification")
    void HideNotification();

    UFUNCTION(BlueprintCallable, Category = "Notification")
    void QueueNotification(const FString& Message, ENotificationType Type);

    UFUNCTION(BlueprintCallable, Category = "Notification")
    void ClearNotificationQueue();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Notification")
    bool IsShowing() const { return bIsShowing; }

    UFUNCTION(BlueprintPure, Category = "Notification")
    int32 GetQueueLength() const { return NotificationQueue.Num(); }

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Notification")
    void OnNotificationShown(const FString& Message, ENotificationType Type);

    UFUNCTION(BlueprintImplementableEvent, Category = "Notification")
    void OnNotificationHidden();

    // Style customization
    UFUNCTION(BlueprintCallable, Category = "Notification")
    void SetNotificationStyle(ENotificationType Type, const FNotificationStyle& Style);

    UFUNCTION(BlueprintPure, Category = "Notification")
    FNotificationStyle GetNotificationStyle(ENotificationType Type) const;
};