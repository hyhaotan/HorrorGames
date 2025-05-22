#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ItemInfoWidget.generated.h"

class AItem;

UCLASS()
class HORRORGAME_API UItemInfoWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    void InitializeWithItem(AItem* InItem);

private:
    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* RootPanel;

    UPROPERTY(meta = (BindWidget))
    UImage* ItemIconImage;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ItemNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ItemDescText;

    static const FVector2D NameOffset;
    static const FVector2D DescOffset;
};
