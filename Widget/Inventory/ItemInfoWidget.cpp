#include "HorrorGame/Widget/Inventory/ItemInfoWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/Actor/Item.h"

const FVector2D UItemInfoWidget::NameOffset = FVector2D(70, 0);
const FVector2D UItemInfoWidget::DescOffset = FVector2D(70, 24);

void UItemInfoWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 1) Xóa root mặc định (nếu có) và tạo mới
    if (WidgetTree->RootWidget)
    {
        WidgetTree->RootWidget->RemoveFromParent();
        WidgetTree->RootWidget = nullptr;
    }

    // 2) Tạo root canvas
    RootPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootPanel"));
    WidgetTree->RootWidget = RootPanel;

    // 3) Tạo widget con
    ItemIconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ItemIconImage"));
    ItemNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ItemNameText"));
    ItemDescText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ItemDescText"));

    // 4) Add vào canvas
    if (UCanvasPanelSlot* IconSlot = RootPanel->AddChildToCanvas(ItemIconImage))
    {
        IconSlot->SetPosition(FVector2D(0, 0));
        IconSlot->SetSize(FVector2D(64, 64));
    }
    if (UCanvasPanelSlot* NameSlot = RootPanel->AddChildToCanvas(ItemNameText))
    {
        NameSlot->SetPosition(NameOffset);
    }
    if (UCanvasPanelSlot* DescSlot = RootPanel->AddChildToCanvas(ItemDescText))
    {
        DescSlot->SetPosition(DescOffset);
    }
}

void UItemInfoWidget::InitializeWithItem(AItem* InItem)
{
    if (!InItem || !InItem->ItemData)
        return;

    if (ItemIconImage)
    {
        if (UTexture2D* IconTex = InItem->ItemData->ItemTextData.Icon)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(IconTex);
            Brush.ImageSize = FVector2D(64, 64);
            ItemIconImage->SetBrush(Brush);
        }
        else
        {
            ItemIconImage->SetBrush(FSlateNoResource());
        }
    }

    if (ItemNameText)
    {
        ItemNameText->SetText(InItem->ItemData->ItemTextData.Name);
    }

    if (ItemDescText)
    {
        ItemDescText->SetText(InItem->ItemData->ItemTextData.Description);
    }
}

