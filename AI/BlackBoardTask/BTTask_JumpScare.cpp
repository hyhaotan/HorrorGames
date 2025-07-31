#include "BTTask_JumpScare.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HorrorGame/AI/NPC_AIController.h"
#include "HorrorGame/AI/NPC.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UBTTask_JumpScare::UBTTask_JumpScare()
{
    NodeName = TEXT("JumpScare Player");
}

EBTNodeResult::Type UBTTask_JumpScare::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    CachedOwnerComp = &OwnerComp;

    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        bool bCanJump = BB->GetValueAsBool(TEXT("bCanJumpScare"));
        if (!bCanJump)
        {
            UE_LOG(LogTemp, Warning, TEXT("JumpScare: Không thể thực hiện jumpscare lúc này."));
            return EBTNodeResult::Failed;
        }
    }

    // Lấy AI Controller
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        return EBTNodeResult::Failed;
    }

    // Lấy NPC (chính Pawn mà AI đang điều khiển)
    ANPC* NPC = Cast<ANPC>(AICon->GetPawn());
    if (!NPC)
    {
        return EBTNodeResult::Failed;
    }

    // Tìm tất cả players gần NPC
    TArray<AHorrorGameCharacter*> NearbyPlayers = GetNearbyPlayers(NPC->GetActorLocation());

    if (NearbyPlayers.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("JumpScare: Không có player nào gần đủ để jumpscare."));
        return EBTNodeResult::Failed;
    }

    // Lọc ra những players không bị knockdown
    TArray<AHorrorGameCharacter*> ValidPlayers;
    for (AHorrorGameCharacter* Player : NearbyPlayers)
    {
        if (Player && !Player->IsKnockedDown())
        {
            ValidPlayers.Add(Player);
        }
    }

    if (ValidPlayers.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("JumpScare: Tất cả players gần đây đều đang bị knockdown."));
        return EBTNodeResult::Failed;
    }

    // Chạy Animation Montage trên NPC (chỉ trên server, sẽ replicate)
    if (JumpScareMontage && NPC->HasAuthority())
    {
        NPC->PlayAnimMontage(JumpScareMontage);
    }

    // Play sound effect (trên server sẽ replicate đến tất cả clients)
    if (!bHasPlayedJumpScareSound && JumpscareSound && NPC->HasAuthority())
    {
        FVector SoundLocation = NPC->GetMesh()->DoesSocketExist("head")
            ? NPC->GetMesh()->GetSocketLocation(FName("head"))
            : NPC->GetActorLocation();
        UGameplayStatics::PlaySoundAtLocation(NPC, JumpscareSound, SoundLocation);
        bHasPlayedJumpScareSound = true;
    }

    // Gọi jumpscare cho tất cả players hợp lệ
    for (AHorrorGameCharacter* Player : ValidPlayers)
    {
        if (Player)
        {
            // Gọi RPC đến client của player này để thực hiện jumpscare
            Player->ClientStartJumpScare(NPC, BlendTime, JumpScareDuration);
        }
    }

    // Bật cờ để NPC biết sẽ trả về góc nhìn từ JumpScareCamera
    NPC->bUseJumpScareCamera = true;

    // Dùng Timer để chờ JumpScareDuration, sau đó kết thúc jumpscare
    if (UWorld* World = OwnerComp.GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TimerHandle_JumpScare,
            this,
            &UBTTask_JumpScare::OnJumpScareComplete,
            JumpScareDuration,
            false
        );
    }

    // Trả về InProgress vì ta sẽ kết thúc task khi timer xong
    return EBTNodeResult::InProgress;
}

void UBTTask_JumpScare::OnJumpScareComplete()
{
    UWorld* World = GetWorld();
    if (!World || !CachedOwnerComp)
    {
        return;
    }

    // 1) Clear timer
    World->GetTimerManager().ClearTimer(TimerHandle_JumpScare);

    // 2) Lấy NPC
    ANPC* NPC = nullptr;
    if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
    {
        NPC = Cast<ANPC>(AICon->GetPawn());
    }

    // 3) Tìm tất cả players gần NPC và kết thúc jumpscare cho họ
    if (NPC)
    {
        TArray<AHorrorGameCharacter*> NearbyPlayers = GetNearbyPlayers(NPC->GetActorLocation());

        for (AHorrorGameCharacter* Player : NearbyPlayers)
        {
            if (Player)
            {
                // Gọi RPC để kết thúc jumpscare và gây knockdown
                Player->ClientEndJumpScare(BlendTime);

                // Gây knockdown (chỉ trên server)
                if (Player->HasAuthority())
                {
                    Player->ServerStartKnockDown();
                }
            }
        }

        // 4) Tắt flag sử dụng camera đặc biệt trên NPC
        NPC->bUseJumpScareCamera = false;
    }

    // 5) Reset blackboard keys
    ResetBlackboardKeys();

    // 6) Restart AI logic
    RestartAILogic();

    // 7) Thông báo hoàn thành task
    FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}

void UBTTask_JumpScare::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type Result)
{
    // Xóa timer nếu task bị hủy hoặc kết thúc sớm
    if (UWorld* World = OwnerComp.GetWorld())
    {
        World->GetTimerManager().ClearTimer(TimerHandle_JumpScare);
    }

    // Reset sound flag cho lần jump scare tiếp theo
    bHasPlayedJumpScareSound = false;

    Super::OnTaskFinished(OwnerComp, NodeMemory, Result);
}

void UBTTask_JumpScare::ResetBlackboardKeys()
{
    if (UBlackboardComponent* BB = CachedOwnerComp->GetBlackboardComponent())
    {
        // Đặt lại các flags jumpscare
        BB->SetValueAsBool(TEXT("bCanJumpScare"), false);
        BB->SetValueAsBool(TEXT("bCantJumpScare"), true);

        // Clear player location để AI không tiếp tục chase
        BB->ClearValue(TEXT("TargetLocation"));

        // Reset về trạng thái patrol bằng cách clear các keys liên quan đến chase
        BB->ClearValue(TEXT("CanSeePlayer"));

        // Đảm bảo có PatrolPathPoint để AI biết đi đâu
        if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
        {
            if (ANPC* NPC = Cast<ANPC>(AICon->GetPawn()))
            {
                int32 CurrentIndex = BB->GetValueAsInt(TEXT("PatrolPathIndex"));

                if (NPC->GetPatrolPath())
                {
                    auto const Point = NPC->GetPatrolPath()->GetPatrolPoint(CurrentIndex);
                    auto const GlobalPoint = NPC->GetPatrolPath()->GetActorTransform().TransformPosition(Point);
                    BB->SetValueAsVector(TEXT("PatrolPathPoint"), GlobalPoint);

                    UE_LOG(LogTemp, Log, TEXT("JumpScare: Set patrol point %d for AI to continue patrol"), CurrentIndex);
                }
            }
        }

        UE_LOG(LogTemp, Log, TEXT("JumpScare: Reset blackboard keys để AI quay về patrol"));
    }
}

void UBTTask_JumpScare::RestartAILogic()
{
    if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
    {
        AICon->GetBrainComponent()->RestartLogic();
        UE_LOG(LogTemp, Warning, TEXT("JumpScare: Restarted AI logic"));
    }
}

TArray<AHorrorGameCharacter*> UBTTask_JumpScare::GetNearbyPlayers(const FVector& NPCLocation) const
{
    TArray<AHorrorGameCharacter*> NearbyPlayers;

    if (UWorld* World = GetWorld())
    {
        // Tìm tất cả players trong game
        for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            if (APlayerController* PC = Iterator->Get())
            {
                if (AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(PC->GetPawn()))
                {
                    float Distance = FVector::Dist(NPCLocation, Player->GetActorLocation());
                    if (Distance <= MaxJumpscareDistance)
                    {
                        NearbyPlayers.Add(Player);
                    }
                }
            }
        }
    }

    return NearbyPlayers;
}