#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "SanityWidget.generated.h"

UCLASS()
class HORRORGAME_API USanityWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    /** Cập nhật phần trăm thanh Sanity */
    UFUNCTION()
    void SetSanityPercent(float Percent);

protected:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* SanityProgressBar;
};
