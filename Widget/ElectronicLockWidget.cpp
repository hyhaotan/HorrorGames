#include "HorrorGame/Widget/ElectronicLockWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "HorrorGame/Actor/ElectronicLockActor.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "TimerManager.h"

bool UElectronicLockWidget::Initialize()
{
    if (!Super::Initialize())
        return false;

    // Bind each digit button dynamically to its wrapper
    if (DigitButton0) DigitButton0->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit0Clicked);
    if (DigitButton1) DigitButton1->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit1Clicked);
    if (DigitButton2) DigitButton2->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit2Clicked);
    if (DigitButton3) DigitButton3->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit3Clicked);
    if (DigitButton4) DigitButton4->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit4Clicked);
    if (DigitButton5) DigitButton5->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit5Clicked);
    if (DigitButton6) DigitButton6->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit6Clicked);
    if (DigitButton7) DigitButton7->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit7Clicked);
    if (DigitButton8) DigitButton8->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit8Clicked);
    if (DigitButton9) DigitButton9->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDigit9Clicked);
    if (CancelButton) CancelButton->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnCancelClicked);
    if (DecreseButton) DecreseButton->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnDecreseClicked);

    if (ClearButton)
        ClearButton->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnClearClicked);
    if (EnterButton)
        EnterButton->OnClicked.AddDynamic(this, &UElectronicLockWidget::OnEnterClicked);

    return true;
}

void UElectronicLockWidget::BindLockActor(AElectronicLockActor* LockActor)
{
    BoundLock = LockActor;

    if (CodeText)
    {
        CodeText->SetText(FText::GetEmpty());
		CodeText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    }

    if (BoundLock)
    {
        BoundLock->OnCodeUpdated.AddDynamic(this, &UElectronicLockWidget::UpdateCodeDisplay);
        BoundLock->OnCodeError.AddDynamic(this, &UElectronicLockWidget::ShowError);
    }
}

void UElectronicLockWidget::OnDigitClicked(int32 Digit)
{
    if (BoundLock)
    {
        BoundLock->AddDigit(Digit);
    }
}

// Wrappers for dynamic binding
void UElectronicLockWidget::OnDigit0Clicked() { OnDigitClicked(0); }
void UElectronicLockWidget::OnDigit1Clicked() { OnDigitClicked(1); }
void UElectronicLockWidget::OnDigit2Clicked() { OnDigitClicked(2); }
void UElectronicLockWidget::OnDigit3Clicked() { OnDigitClicked(3); }
void UElectronicLockWidget::OnDigit4Clicked() { OnDigitClicked(4); }
void UElectronicLockWidget::OnDigit5Clicked() { OnDigitClicked(5); }
void UElectronicLockWidget::OnDigit6Clicked() { OnDigitClicked(6); }
void UElectronicLockWidget::OnDigit7Clicked() { OnDigitClicked(7); }
void UElectronicLockWidget::OnDigit8Clicked() { OnDigitClicked(8); }
void UElectronicLockWidget::OnDigit9Clicked() { OnDigitClicked(9); }

void UElectronicLockWidget::OnClearClicked()
{
    if (BoundLock)
    {
        BoundLock->ClearEnteredCode();
        if (CodeText) CodeText->SetText(FText::GetEmpty());
    }
}

void UElectronicLockWidget::OnEnterClicked()
{
    if (BoundLock)
        BoundLock->VerifyCode();
}

void UElectronicLockWidget::OnCancelClicked()
{
	AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(GetOwningPlayerPawn());
    BoundLock->EnableMovementPlayer(Player,true);
}

void UElectronicLockWidget::OnDecreseClicked()
{
    if (BoundLock)
    {
        BoundLock->DecreaseCode();
		UpdateCodeDisplay(BoundLock->EnteredCode);
    }
}

void UElectronicLockWidget::UpdateCodeDisplay(const TArray<int32>& CurrentCode)
{
    FString Display;
    for (int32 Num : CurrentCode)
        Display.AppendInt(Num);
    if (CodeText)
        CodeText->SetText(FText::FromString(Display));
}

void UElectronicLockWidget::ShowError()
{
    if (!CodeText || !GetWorld()) return;
    CodeText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [this]() { if (CodeText) CodeText->SetColorAndOpacity(FSlateColor(FLinearColor::White)); },
        1.0f,
        false
    );
}
