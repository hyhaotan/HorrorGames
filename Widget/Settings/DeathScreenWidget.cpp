#include "DeathScreenWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UDeathScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind sự kiện cho nút Restart nếu đã được gán từ Widget Blueprint
    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UDeathScreenWidget::OnRestartClicked);
    }

    // Bind sự kiện cho nút Quit nếu đã được gán từ Widget Blueprint
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UDeathScreenWidget::OnQuitClicked);
    }
}

void UDeathScreenWidget::OnRestartClicked()
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Lấy tên level hiện tại và mở lại để chơi lại
        FName CurrentLevel = *UGameplayStatics::GetCurrentLevelName(World);
        UGameplayStatics::OpenLevel(World, CurrentLevel);
    }
}

void UDeathScreenWidget::OnQuitClicked()
{
    // Lấy Player Controller và thực hiện lệnh thoát game
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->ConsoleCommand("quit");
    }
}
