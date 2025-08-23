#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HorrorGameHUD.generated.h"

UCLASS()
class HORRORGAME_API AHorrorGameHUD : public AHUD
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowNotification(const FText& Message, float Duration = 3.0f);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UNotificationWidget> NotificationWidgetClass;

private:
    UPROPERTY()
    class UNotificationWidget* NotificationWidget;
};