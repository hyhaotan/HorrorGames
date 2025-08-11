#include "SanityWidget.h"
#include "Components/ProgressBar.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"

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
	FTimerHandle TimerHandle;

    Percent = FMath::Clamp(Percent, 0.f, 1.f);
    SanityProgressBar->SetPercent(Percent);

    // Đổi màu theo mức độ
    if (Percent > 0.75f)
        SanityProgressBar->SetFillColorAndOpacity(FLinearColor::Green);
    else if (Percent > 0.5f)
        SanityProgressBar->SetFillColorAndOpacity(FLinearColor::Yellow);
    else if (Percent > 0.25f)
        SanityProgressBar->SetFillColorAndOpacity(FLinearColor(255.f, 99.f, 71.f));
    else
    {
        SanityProgressBar->SetFillColorAndOpacity(FLinearColor::Red);
        PlayAnimSanity();
        GetWorld()->GetTimerManager().SetTimer(TimerHandle,0.2f,true);
    }
}
