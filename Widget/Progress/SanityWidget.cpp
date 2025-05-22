#include "SanityWidget.h"
#include "Components/ProgressBar.h"
#include "HorrorGame/HorrorGameCharacter.h"

void USanityWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Khi widget vừa construct, khởi tạo bằng giá trị hiện tại của player
    if (SanityProgressBar)
    {
        if (APawn* P = GetOwningPlayerPawn())
        {
            if (AHorrorGameCharacter* C = Cast<AHorrorGameCharacter>(P))
            {
                SanityProgressBar->SetPercent(C->Sanity / C->MaxSanity);
            }
        }
    }
}

void USanityWidget::SetSanityPercent(float Percent)
{
    if (!SanityProgressBar) return;

    Percent = FMath::Clamp(Percent, 0.f, 1.f);
    SanityProgressBar->SetPercent(Percent);

    // Đổi màu theo mức độ
    if (Percent > 0.7f)
        SanityProgressBar->SetFillColorAndOpacity(FLinearColor::Red);
    else if (Percent > 0.3f)
        SanityProgressBar->SetFillColorAndOpacity(FLinearColor::Yellow);
    else
        SanityProgressBar->SetFillColorAndOpacity(FLinearColor::Green);
}
