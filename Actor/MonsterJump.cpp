#include "MonsterJump.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Widget/Progress/ProgressBarWidget.h"
#include "HorrorGame/AI/NPC_AIController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "InputCoreTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h" 


AMonsterJump::AMonsterJump()
{
    PrimaryActorTick.bCanEverTick = true;
    TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
    TriggerZone->SetupAttachment(RootComponent);
    TriggerZone->SetBoxExtent(FVector(100.f));
    TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
    TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AMonsterJump::OnOverlapBegin);

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = ANPC_AIController::StaticClass();

    EscapeProgress = 0.f;
    bIsGrabbing = false;
    TotalHits = TotalMisses = 0;
    MissCount = 0;
}

void AMonsterJump::BeginPlay()
{
    Super::BeginPlay();

    if (BehaviorTreeAsset)
    {
        if (auto* MyAI = Cast<ANPC_AIController>(GetController()))
        {
            UBlackboardComponent* BBComp = nullptr;
            MyAI->UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BBComp);
            MyAI->RunBehaviorTree(BehaviorTreeAsset);
        }
    }
    else
    {
        // Ví dụ fallback: chase player
        if (auto* MyAI = Cast<ANPC_AIController>(GetController()))
        {
            if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
            {
                MyAI->MoveToActor(Player, /*AcceptanceRadius=*/100.f);
            }
        }
    }
}

void AMonsterJump::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsGrabbing && CapturedPlayer)
    {
        CapturedPlayer->RecoverSanity(DeltaTime * 1.f);
        if (EscapeProgress > 0.f)
        {
            EscapeProgress = FMath::Max(0.f, EscapeProgress - DeltaTime * 0.05f);
            UpdateWidget();
        }
    }
}

void AMonsterJump::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(OtherActor);
    if (!Player) return;

    InitializeGrabbedPlayer(Player);
}

void AMonsterJump::StartQTE(bool bClearProgress)
{
    if (bClearProgress) EscapeProgress = 0.f;
    AdjustDifficulty();

    ChooseRandomPhase();
    GenerateSequenceByPhase();
    CurrentQTEIndex = 0;
    UpdateWidget();
}

void AMonsterJump::AdjustDifficulty()
{
    float Rate = (TotalHits + TotalMisses > 0) ? (float)TotalHits / (TotalHits + TotalMisses) : 0.5f;
    UE_LOG(LogTemp, Log, TEXT("[AdjustDifficulty] Rate=%.2f, SeqLen(before)=%d, AllowedTime(before)=%.2f"), Rate, SequenceLength, AllowedInputTime);

    if (Rate > 0.8f)
    {
        SequenceLength = FMath::Clamp(SequenceLength + 1, 2, 8);
        AllowedInputTime = FMath::Max(AllowedInputTime - 0.1f, 0.5f);
    }
    else if (Rate < 0.5f)
    {
        SequenceLength = FMath::Clamp(SequenceLength - 1, 2, 8);
        AllowedInputTime += 0.1f;
    }
    PerfectThreshold = AllowedInputTime * 0.3f;
    GoodThreshold = AllowedInputTime * 0.7f;

    UE_LOG(LogTemp, Log, TEXT("[AdjustDifficulty] SeqLen(after)=%d, AllowedTime(after)=%.2f"), SequenceLength, AllowedInputTime);
    TotalHits = TotalMisses = 0;
}

EQTEResult AMonsterJump::EvaluateTiming(float DeltaFromTarget) const
{
    float AbsDelta = FMath::Abs(DeltaFromTarget);
    if (AbsDelta <= PerfectThreshold) return EQTEResult::Perfect;
    if (AbsDelta <= GoodThreshold)    return EQTEResult::Good;
    return EQTEResult::Miss;
}

void AMonsterJump::NextQTESequence()
{
    AdjustDifficulty();
    GenerateSequenceByPhase();
    CurrentQTEIndex = 0;
    UpdateWidget();
}

void AMonsterJump::ReceiveEscapeInput(FKey PressedKey)
{
    if (!bIsGrabbing || bIsStunned) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeDelta = CurrentTime - LastPromptTime;
    EQTEResult Res = EvaluateTiming(TimeDelta);

    UE_LOG(LogTemp, Log, TEXT("[QTE] Key=%s, Delta=%.3f, Result=%s"), *PressedKey.GetFName().ToString(), TimeDelta,
        *UEnum::GetValueAsString(Res));

    EscapeWidget->PlayPress();
    if (QTESequence.IsValidIndex(CurrentQTEIndex) &&
        PressedKey == QTESequence[CurrentQTEIndex] &&
        Res != EQTEResult::Miss)
    {
        float Mult = (Res == EQTEResult::Perfect) ? 1.5f : 1.0f;
        EscapeProgress += IncrementPerStep * Mult;
        TotalHits++;

        if (EscapeProgress >= EscapeTarget)
        {
            CompleteEscape();
            return;
        }

        CurrentQTEIndex++;
        if (CurrentQTEIndex >= QTESequence.Num())
        {
            NextQTESequence();
            return;
        }
    }
    else
    {
        MissCount++;
        TotalMisses++;
        if (MissCount >= 3)
        {
            ApplyStun(3.f);
            MissCount = 0;
        }
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
        UTexture2D* Icon = GetKeyIconTexture(QTESequence[CurrentQTEIndex]);
        EscapeWidget->SetNextKeyImage(Icon);
    }

    UTexture2D* PhaseTex = GetPhaseIconTexture(CurrentPhase);
    EscapeWidget->SetPhaseImage(PhaseTex);

    LastPromptTime = GetWorld()->GetTimeSeconds();
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
        CapturedPlayer->ResumeSanityDrain();
        CapturedPlayer->bIsGrabbed = false;
        CapturedPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        CapturedPlayer->ClearGrabbingMonster();
        CapturedPlayer->EnableFirstPerson();
        CapturedPlayer = nullptr;
    }
    Destroy();
}

void AMonsterJump::ApplyStun(float Duration)
{
    if (!CapturedPlayer) return;
    CapturedPlayer->GetCharacterMovement()->DisableMovement();
    bIsStunned = true;

    if (StunMontage)
    {
        if (UAnimInstance* Anim = CapturedPlayer->GetMesh()->GetAnimInstance())
        {
            Anim->Montage_Play(StunMontage);
        }
    }

    if (APlayerController* PC = Cast<APlayerController>(CapturedPlayer->GetController()))
    {
        DisableInput(PC);
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
        PC->PlayerCameraManager->StartCameraShake(StunCameraShake);
    }

    UGameplayStatics::PlaySoundAtLocation(CapturedPlayer, StunSound, CapturedPlayer->GetMesh()->GetSocketLocation("head"));

    GetWorldTimerManager().SetTimer(StunTimerHandle, this, &AMonsterJump::ReleaseStun, Duration, false);
}

void AMonsterJump::ReleaseStun()
{
    if (!CapturedPlayer) return;
    bIsStunned = false;

    if (StunReverseMontage)
    {
        if (UAnimInstance* Anim = CapturedPlayer->GetMesh()->GetAnimInstance())
        {
            Anim->Montage_Play(StunReverseMontage);
        }
    }

    if (APlayerController* PC = Cast<APlayerController>(CapturedPlayer->GetController()))
    {
        EnableInput(PC);
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
    }
}

void AMonsterJump::InitializeGrabbedPlayer(AHorrorGameCharacter* Player)
{
    AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("head"));
    Player->EnableThirdPerson();
    Player->GetCharacterMovement()->DisableMovement();
    Player->SetGrabbingMonster(this);

    if (ProgressBarClass)
    {
        EscapeWidget = CreateWidget<UProgressBarWidget>(GetWorld(), ProgressBarClass);
        if (EscapeWidget) EscapeWidget->AddToViewport();
    }

    CapturedPlayer = Player;
    bIsGrabbing = true;
    CapturedPlayer->bIsGrabbed = true;
    bPhaseInitialized = false;

    StartQTE(true);
}

UTexture2D* AMonsterJump::GetKeyIconTexture(const FKey& Key) const
{
    //cách 1 dùng TMap
    const FName Name = Key.GetFName();
    if (const auto* SoftPtr = KeyIcons.Find(Name))
    {
        return SoftPtr->LoadSynchronous();
    }
    return nullptr;

	//cách 2 dùng DataTable
    //if (!KeyIconTable) return nullptr;

    //static const FString Context = TEXT("GetKeyIconTexture");
    //FName RowName = Key.GetFName();

    //FQTEKeyIconRow* Row = KeyIconTable->FindRow<FQTEKeyIconRow>(RowName, Context);
    //if (Row && Row->Icon.IsValid())
    //{
    //    return Row->Icon.LoadSynchronous();
    //}
    //return nullptr;
}

UTexture2D* AMonsterJump::GetPhaseIconTexture(EQTEPhase Phase) const
{
	//cách 1 dùng TMap
    if (const auto* SoftPtr = PhaseIcons.Find(Phase))
    {
        return SoftPtr->LoadSynchronous();
    }
    return nullptr;

	//cách 2 dùng DataTable
   /* if (!PhaseIconTable) return nullptr;

    static const FString Context = TEXT("GetPhaseIconTexture");

    for (auto& Pair : PhaseIconTable->GetRowMap())
    {
        FQTEPhaseIconRow* Row = (FQTEPhaseIconRow*)Pair.Value;
        if (Row && Row->Phase == Phase)
        {
            if (Row->Icon.IsValid())
                return Row->Icon.LoadSynchronous();
        }
    }
    return nullptr;*/
}

void AMonsterJump::ChooseRandomPhase()
{
    int32 Pick = FMath::RandRange(0, 1);
    CurrentPhase = static_cast<EQTEPhase>(Pick);
}

void AMonsterJump::GenerateSequenceByPhase()
{
    QTESequence.Empty();
    TArray<FKey> Pool;

    switch (CurrentPhase)
    {
    case EQTEPhase::WASD:
        Pool = { EKeys::W, EKeys::A, EKeys::S, EKeys::D };
        break;
    case EQTEPhase::Arrows:
        Pool = { EKeys::Up, EKeys::Down, EKeys::Left, EKeys::Right };
        break;
    }

    for (int32 i = 0; i < SequenceLength; ++i)
        QTESequence.Add(Pool[FMath::RandRange(0, Pool.Num() - 1)]);
}