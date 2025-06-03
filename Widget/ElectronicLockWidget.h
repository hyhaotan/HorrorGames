#pragma once

#include "Blueprint/UserWidget.h"
#include "ElectronicLockWidget.generated.h"

class UButton;
class UTextBlock;
class AElectronicLockActor;
                          
UCLASS()
class HORRORGAME_API UElectronicLockWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual bool Initialize() override;

    /** Binds this widget to the lock actor */
    void BindLockActor(AElectronicLockActor* LockActor);

    /** Flash error color */
    UFUNCTION()
    void ShowError();

    /** Update the displayed code */
    UFUNCTION()
    void UpdateCodeDisplay(const TArray<int32>& CurrentCode);

protected:
    /** Core digit handler */
    void OnDigitClicked(int32 Digit);

    /** Generated wrappers for dynamic binding */
    UFUNCTION()
    void OnDigit0Clicked();
    UFUNCTION()
    void OnDigit1Clicked();
    UFUNCTION()
    void OnDigit2Clicked();
    UFUNCTION()
    void OnDigit3Clicked();
    UFUNCTION()
    void OnDigit4Clicked();
    UFUNCTION()
    void OnDigit5Clicked();
    UFUNCTION()
    void OnDigit6Clicked();
    UFUNCTION()
    void OnDigit7Clicked();
    UFUNCTION()
    void OnDigit8Clicked();
    UFUNCTION()
    void OnDigit9Clicked();

    /** Callback for clear button */
    UFUNCTION()
    void OnClearClicked();

    /** Callback for enter button */
    UFUNCTION()
    void OnEnterClicked(); 
    
    UFUNCTION()
    void OnCancelClicked();

    UFUNCTION()
    void OnDecreseClicked();

    /** Digit buttons 0-9 */
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton0;
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton1;
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton2;
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton3;
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton4;
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton5;
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton6;
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton7;
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton8;
    UPROPERTY(meta = (BindWidget)) UButton* DigitButton9;

    /** Other controls */
    UPROPERTY(meta = (BindWidget)) UButton* ClearButton;
    UPROPERTY(meta = (BindWidget)) UButton* EnterButton;
    UPROPERTY(meta = (BindWidget)) UButton* CancelButton;
    UPROPERTY(meta = (BindWidget)) UButton* DecreseButton;
    UPROPERTY(meta = (BindWidget)) UTextBlock* CodeText;

private:
    /** Currently bound lock actor */
    AElectronicLockActor* BoundLock = nullptr;
};