#include "MonsterJump.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HorrorGame/Widget/ProgressBarWidget.h"

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
    // Optionally decay progress over time
    if (bIsGrabbing && EscapeProgress > 0.f)
    {
        EscapeProgress = FMath::Max(0.f, EscapeProgress - DeltaTime * 0.05f);
        if (EscapeWidget)
        {
            EscapeWidget->SetProgressPercent(EscapeProgress / EscapeTarget);
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
    // Only respond to the player character
    AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    // 1) Monster jumps up
    LaunchCharacter(FVector(0.f, 0.f, 600.f), false, true);

    // 2) Attach monster to player's head socket after a short delay
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, false);
    AttachToComponent(Player->GetMesh(), AttachRules, TEXT("head"));

    // 3) Switch player's camera to third-person
    Player->EnableThirdPerson();

    if (ProgressBarClass)
    {
        EscapeWidget = CreateWidget<UProgressBarWidget>(GetWorld(), ProgressBarClass);
        if (EscapeWidget)
        {
            EscapeWidget->AddToViewport();
            EscapeWidget->SetProgressPercent(0.f);
        }
    }

    CapturedPlayer = Player;
    Player->GetCharacterMovement()->DisableMovement();

    bIsGrabbing = true;
    // Inform player character who is grabbing
    Player->SetGrabbingMonster(this);
}

void AMonsterJump::ReceiveEscapeInput()
{
    if (!bIsGrabbing)
        return;

    EscapeProgress += IncrementPerPress;
    EscapeProgress = FMath::Clamp(EscapeProgress, 0.f, EscapeTarget);
    if (EscapeWidget)
    {
        EscapeWidget->SetProgressPercent(EscapeProgress / EscapeTarget);
    }

    if (EscapeProgress >= EscapeTarget)
    {
        // Detach monster
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        if (EscapeWidget) { EscapeWidget->RemoveFromParent(); EscapeWidget = nullptr; }
        bIsGrabbing = false;
        EscapeProgress = 0.f;

        if (CapturedPlayer)
        {
            CapturedPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            CapturedPlayer->ClearGrabbingMonster();
            CapturedPlayer->EnableFirstPerson();
            CapturedPlayer = nullptr;
        }
    }
}