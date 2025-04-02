#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathScreenWidget.generated.h"

/**
 *  Widget hiển thị khi người chơi chết
 */
UCLASS()
class HORRORGAME_API UDeathScreenWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Hàm khởi tạo widget
    virtual void NativeConstruct() override;

protected:
    // Bind các thành phần UI từ UMG (thiết kế trong Widget Blueprint)
    UPROPERTY(meta = (BindWidget))
    class UButton* RestartButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* QuitButton;

    // Xử lý khi nút Restart được bấm
    UFUNCTION()
    void OnRestartClicked();

    // Xử lý khi nút Quit được bấm
    UFUNCTION()
    void OnQuitClicked();
};
