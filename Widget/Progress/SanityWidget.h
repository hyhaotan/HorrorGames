#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Animation/WidgetAnimation.h"
#include "SanityWidget.generated.h"

class UWidgetAnimation;

UCLASS()
class HORRORGAME_API USanityWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    /** Cập nhật phần trăm thanh Sanity */
    UFUNCTION()
    void SetSanityPercent(float Percent);

    UPROPERTY(meta = (BindWidgetAnim),Transient)
    UWidgetAnimation* StartAnim;

    UPROPERTY(meta = (BindWidgetAnim),Transient)
    UWidgetAnimation* EndAnim;

    UFUNCTION()
    void PlayAnimSanity() { PlayAnimation(StartAnim); }

    UFUNCTION()
    void StopAnimSanity() { PlayAnimation(EndAnim); }


protected:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* SanityProgressBar;
};
