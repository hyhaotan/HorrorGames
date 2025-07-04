#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyNotificationWidget.generated.h"

class UTextBlock;

UCLASS()
class HORRORGAME_API UKeyNotificationWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Cập nhật text và show widget */
    UFUNCTION(BlueprintCallable, Category = "Key Notification")
    void UpdateKeyNotification(const FString& KeyName);

protected:
    /** Bind widget trong UMG */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* KeyNotificationText;

	void SetDisplayText();
};
