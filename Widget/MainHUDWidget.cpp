#include "MainHUDWidget.h"

bool UMainHUDWidget::Initialize()
{
    if (!Super::Initialize())
        return false;

    // Gom pointer nào != null vào mảng
    if (PlayerHUDPanel)    Panels.Add(PlayerHUDPanel);
    if (InventoryPanel)    Panels.Add(InventoryPanel);
    if (InventorySlotPanel)Panels.Add(InventorySlotPanel);

    // Mặc định ẩn hết
    SetAllPanelsVisibility(false);
    return true;
}

void UMainHUDWidget::SetAllPanelsVisibility(bool bVisible)
{
    const ESlateVisibility V = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
    for (UWidget* W : Panels)
    {
        W->SetVisibility(V);
    }
}

void UMainHUDWidget::ShowOnlyPanel(FName PanelName)
{
    // So sánh tên widget với PanelName rồi show, còn lại ẩn
    for (UWidget* W : Panels)
    {
        bool bMatch = W->GetFName() == PanelName;
        W->SetVisibility(bMatch ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}
