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
	virtual void NativeConstruct() override;

	// Hàm cập nhật dữ liệu widget
	UFUNCTION(BlueprintCallable, Category = "Item Widget")
	void SetItemData(const FItemTextData& NewData);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemNameText;
};