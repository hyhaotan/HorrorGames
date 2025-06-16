#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUDWidget.generated.h"

/**
 * Widget chủ quản cho mọi panel (HUD, Inventory, Slot, v.v.)
 */
UCLASS()
class HORRORGAME_API UMainHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual bool Initialize() override;

protected:
    // Các sub-panel được bind từ UMG (BindWidgetOptional để nếu không có thì không crash)
    UPROPERTY(meta = (BindWidgetOptional))
    UWidget* PlayerHUDPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    UWidget* InventoryPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    UWidget* InventorySlotPanel;

    // Mảng gom tất cả panel để thao tác chung
    TArray<UWidget*> Panels;

public:
    /** Set tất cả panel Visible/Hidden chỉ với 1 lời gọi */
    UFUNCTION(BlueprintCallable, Category = "MainHUD")
    void SetAllPanelsVisibility(bool bVisible);

    /** Chỉ show đúng 1 panel, auto ẩn hết panel khác */
    UFUNCTION(BlueprintCallable, Category = "MainHUD")
    void ShowOnlyPanel(FName PanelName);
};
