#include "HorrorGame/Actor/CountDown.h"
#include "Components/TextRenderComponent.h"
#include "Blueprint/UserWidget.h" // Thêm include này cho UUserWidget

// Sets default values
ACountDown::ACountDown()
{
    PrimaryActorTick.bCanEverTick = false;

    CountdownText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CountdownNumber"));
    CountdownText->SetHorizontalAlignment(EHTA_Center);
    CountdownText->SetWorldSize(150.0f);
    CountdownText->SetupAttachment(RootComponent);

    CountdownTime = 3;
}

// Called when the game starts or when spawned
void ACountDown::BeginPlay()
{
    Super::BeginPlay();

    UpdateTimerDisplay();
    GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ACountDown::AdvanceTimer, 1.0f, true);
}

// Called every frame
void ACountDown::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACountDown::UpdateTimerDisplay()
{
    CountdownText->SetText(FText::FromString(FString::FromInt(FMath::Max(CountdownTime, 0))));
}

void ACountDown::AdvanceTimer()
{
    --CountdownTime;
    UpdateTimerDisplay();
    if (CountdownTime < 1)
    {
        // Dừng timer khi kết thúc đếm ngược.
        GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
        // Thực hiện các hành động cần thiết khi kết thúc đếm ngược.
        CountdownHasFinished();
    }
}

void ACountDown::CountdownHasFinished_Implementation()
{
    // Thay đổi nội dung hiển thị nếu cần (ở đây vẫn giữ “GO!” cho text render).
    CountdownText->SetText(FText::FromString(TEXT("GO!")));

    // Kiểm tra nếu widget jumpscare đã được thiết lập
    if (JumpscareWidgetClass)
    {
        // Tạo instance của widget jumpscare
        UUserWidget* JumpscareWidget = CreateWidget(GetWorld(), JumpscareWidgetClass);
        if (JumpscareWidget)
        {
            // Thêm widget jumpscare vào viewport để hiển thị
            JumpscareWidget->AddToViewport();
        }
    }
}
