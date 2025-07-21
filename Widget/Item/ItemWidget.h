#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HorrorGame/Data/ItemData.h"
#include "ItemWidget.generated.h"

UCLASS()
class HORRORGAME_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // Những animation bạn đặt tên trong UMG: Show và Hide
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ShowAnim;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* HideAnim;

    UFUNCTION(BlueprintCallable)
    void PlayShow() { if (ShowAnim) PlayAnimation(ShowAnim); }

    UFUNCTION(BlueprintCallable)
    void PlayHide() { if (HideAnim) PlayAnimation(HideAnim); }
};