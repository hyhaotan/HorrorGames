#include "MonsterJump.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Widget/ProgressBarWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

AMonsterJump::AMonsterJump()
{
    PrimaryActorTick.bCanEverTick = true;
    TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
    TriggerZone->SetupAttachment(RootComponent);
    TriggerZone->SetBoxExtent(FVector(100.f));
    TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
    TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AMonsterJump::OnOverlapBegin);

    EscapeProgress = 0.f;
    bIsGrabbing = false;
}

void AMonsterJump::BeginPlay()
{
    Super::BeginPlay();
}

void AMonsterJump::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bIsGrabbing)
    {
        // Tăng sanity khi bị quái bắt
        if (CapturedPlayer)
        {
            CapturedPlayer->RecoverSanity(DeltaTime * 3.f); // hồi 3 đơn vị mỗi giây (tùy chỉnh)
        }

        if (EscapeProgress > 0.f)
        {
            EscapeProgress = FMath::Max(0.f, EscapeProgress - DeltaTime * 0.05f);
            UpdateWidget();
        }
    }
}

void AMonsterJump::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(OtherActor);
    if (!Player) return;

    LaunchCharacter(FVector(0, 0, 600), false, true);
    AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("head"));
    Player->EnableThirdPerson();
    Player->GetCharacterMovement()->DisableMovement();
    Player->SetGrabbingMonster(this);

    if (ProgressBarClass)
    {
        EscapeWidget = CreateWidget<UProgressBarWidget>(GetWorld(), ProgressBarClass);
        if (EscapeWidget)
        {
            EscapeWidget->AddToViewport();
        }
    }

    CapturedPlayer = Player;
    bIsGrabbing = true;

    CapturedPlayer->Sanity = 0.f;
    CapturedPlayer->RecoverSanity(0.f);  // refresh widget

    // Khởi tạo QTE và reset tiến trình
    StartQTE(true);
}

void AMonsterJump::StartQTE(bool bClearProgress)
{
    if (bClearProgress)
    {
        EscapeProgress = 0.f;
    }
    QTESequence.Empty();
    TArray<FKey> Pool = { EKeys::W, EKeys::A, EKeys::S, EKeys::D };
    for (int i = 0; i < SequenceLength; ++i)
    {
        QTESequence.Add(Pool[FMath::RandRange(0, Pool.Num() - 1)]);
    }
    CurrentQTEIndex = 0;
    UpdateWidget();
}

void AMonsterJump::NextQTESequence()
{
    QTESequence.Empty();
    TArray<FKey> Pool = { EKeys::W, EKeys::A, EKeys::S, EKeys::D };
    for (int i = 0; i < SequenceLength; ++i)
    {
        QTESequence.Add(Pool[FMath::RandRange(0, Pool.Num() - 1)]);
    }
    CurrentQTEIndex = 0;
    UpdateWidget();
}

void AMonsterJump::ReceiveEscapeInput(FKey PressedKey)
{
    if (!bIsGrabbing) return;

    if (PressedKey == QTESequence[CurrentQTEIndex])
    {
        EscapeProgress += IncrementPerStep;

        if (EscapeProgress >= EscapeTarget)
        {
            CompleteEscape();
            return;
        }

        CurrentQTEIndex++;
        if (CurrentQTEIndex >= QTESequence.Num())
        {
            // Sinh chuỗi mới, không reset progress
            NextQTESequence();
            return;
        }
    }
    else
    {
        EscapeProgress = FMath::Max(0.f, EscapeProgress - IncrementPerStep);
        CurrentQTEIndex = 0;
    }

    UpdateWidget();
}

void AMonsterJump::UpdateWidget()
{
    if (!EscapeWidget) return;
    EscapeWidget->SetProgressPercent(EscapeProgress / EscapeTarget);
    if (QTESequence.IsValidIndex(CurrentQTEIndex))
    {
        EscapeWidget->SetNextKey(QTESequence[CurrentQTEIndex].GetFName().ToString());
    }
}

void AMonsterJump::CompleteEscape()
{
    if (EscapeWidget)
    {
        EscapeWidget->RemoveFromParent();
        EscapeWidget = nullptr;
    }
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    bIsGrabbing = false;
    if (CapturedPlayer)
    {
        CapturedPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        CapturedPlayer->ClearGrabbingMonster();
        CapturedPlayer->EnableFirstPerson();
        CapturedPlayer = nullptr;
    }
}
