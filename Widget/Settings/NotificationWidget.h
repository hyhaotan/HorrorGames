#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotificationWidget.generated.h"

UCLASS()
class HORRORGAME_API UNotificationWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Notification")
    void ShowNotification(const FText& Message, float Duration = 3.0f);

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* NotificationText;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* FadeInOut;

private:
    UFUNCTION()
    void HideNotification();

    FTimerHandle HideTimer;
};