#include "BTTask_JumpScare.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HorrorGame/AI/NPC_AIController.h"
#include "HorrorGame/AI/NPC.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

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

    // Lấy PlayerController
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        return EBTNodeResult::Failed;
    }

    AHorrorGameCharacter* PlayerChar = Cast<AHorrorGameCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (PlayerChar && PlayerChar->IsKnockedDown())
    {
        UE_LOG(LogTemp, Warning, TEXT("JumpScare: Player đang bị knockdown, không thực hiện jumpscare."));
        return EBTNodeResult::Failed;
    }

    // Nếu có Montage, chạy AnimMontage jump scare
    if (JumpScareMontage)
    {
        NPC->PlayAnimMontage(JumpScareMontage);
    }

    if (!bHasPlayedJumpScareSound && JumpscareSound)
    {
        FVector SoundLocation = NPC->GetMesh()->DoesSocketExist("head")
            ? NPC->GetMesh()->GetSocketLocation(FName("head"))
            : NPC->GetActorLocation();
        UGameplayStatics::PlaySoundAtLocation(NPC, JumpscareSound, SoundLocation);
        bHasPlayedJumpScareSound = true;
    }

    // Bật cờ để NPC biết sẽ trả về góc nhìn từ JumpScareCamera
    NPC->bUseJumpScareCamera = true;

    // Chuyển view của người chơi sang NPC (bây giờ NPC chính là ViewTarget)
    PC->SetViewTargetWithBlend(NPC, BlendTime, EViewTargetBlendFunction::VTBlend_Cubic);

    // Dùng Timer để chờ JumpScareDuration, sau đó quay về camera người chơi
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

    // 1) Clear timer phòng trường hợp OnTaskFinished không kịp
    World->GetTimerManager().ClearTimer(TimerHandle_JumpScare);

    // 2) Chuyển view về Pawn người chơi
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC)
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PC->SetViewTargetWithBlend(PlayerPawn, BlendTime, EViewTargetBlendFunction::VTBlend_Cubic);
        }
    }

    // 3) Gây knock‑down cho player ngay sau khi jump scare xong
    if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0))
    {
        if (AHorrorGameCharacter* HorrorChar = Cast<AHorrorGameCharacter>(PlayerChar))
        {
            HorrorChar->StartKnockDown();
        }
    }

    // 4) Tắt flag sử dụng camera đặc biệt trên NPC
    if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
    {
        if (ANPC* NPC = Cast<ANPC>(AICon->GetPawn()))
        {
            NPC->bUseJumpScareCamera = false;
        }
    }

    // 5) Reset các blackboard keys để AI có thể tiếp tục patrol
    if (UBlackboardComponent* BB = CachedOwnerComp->GetBlackboardComponent())
    {
        // Đặt lại các flags jumpscare
        BB->SetValueAsBool(TEXT("bCanJumpScare"), false);
        BB->SetValueAsBool(TEXT("bCantJumpScare"), true);

        // Clear player location để AI không tiếp tục chase
        BB->ClearValue(TEXT("TargetLocation"));

        // Reset về trạng thái patrol bằng cách clear các keys liên quan đến chase
        BB->ClearValue(TEXT("CanSeePlayer"));

        // Reset PatrolPathIndex về 0 hoặc giữ nguyên để tiếp tục patrol
        // BB->SetValueAsInt(TEXT("PatrolPathIndex"), 0); // Uncomment nếu muốn reset về điểm đầu

        // Đảm bảo có PatrolPathPoint để AI biết đi đâu
        if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
        {
            if (ANPC* NPC = Cast<ANPC>(AICon->GetPawn()))
            {
                // Lấy index hiện tại hoặc reset về 0
                int32 CurrentIndex = BB->GetValueAsInt(TEXT("PatrolPathIndex"));

                // Đảm bảo có patrol path
                if (NPC->GetPatrolPath())
                {
                    // Lấy điểm patrol hiện tại
                    auto const Point = NPC->GetPatrolPath()->GetPatrolPoint(CurrentIndex);
                    auto const GlobalPoint = NPC->GetPatrolPath()->GetActorTransform().TransformPosition(Point);
                    BB->SetValueAsVector(TEXT("PatrolPathPoint"), GlobalPoint);

                    UE_LOG(LogTemp, Log, TEXT("JumpScare: Set patrol point %d for AI to continue patrol"), CurrentIndex);
                }
            }
        }

        UE_LOG(LogTemp, Log, TEXT("JumpScare: Reset blackboard keys để AI quay về patrol"));
    }

    // 6) Force restart behavior tree để đảm bảo AI quay về patrol
    if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
    {
        // Restart logic để behavior tree đánh giá lại từ đầu
        AICon->GetBrainComponent()->RestartLogic();
        UE_LOG(LogTemp, Warning, TEXT("JumpScare: Restarted AI logic"));
    }

    // 7) Thông báo hoàn thành task cho Behavior Tree
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