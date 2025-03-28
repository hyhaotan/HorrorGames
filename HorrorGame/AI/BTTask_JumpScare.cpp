#include "HorrorGame/AI/BTTask_JumpScare.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPC_AIController.h"
#include "NPC.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

UBTTask_JumpScare::UBTTask_JumpScare()
{
    NodeName = TEXT("JumpScare Player");
}

EBTNodeResult::Type UBTTask_JumpScare::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // Lưu pointer của BehaviorTreeComponent
    CachedOwnerComp = &OwnerComp;

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

    // Nếu có Montage, chạy AnimMontage jump scare
    if (JumpScareMontage)
    {
        NPC->PlayAnimMontage(JumpScareMontage);
    }

    if (!bHasPlayedJumpScareSound && JumpscareSound)
    {
        // Phát sound tại socket "head" nếu tồn tại, ngược lại dùng vị trí Actor
        FVector SoundLocation = NPC->GetMesh()->DoesSocketExist("head")
            ? NPC->GetMesh()->GetSocketLocation(FName("head"))
            : NPC->GetActorLocation();
        UGameplayStatics::PlaySoundAtLocation(NPC, JumpscareSound, SoundLocation);
        bHasPlayedJumpScareSound = true;
        UE_LOG(LogTemp, Warning, TEXT("JumpScare: Đã phát sound jumpscare."));
    }

    // Bật cờ để NPC biết sẽ trả về góc nhìn từ JumpScareCamera
    NPC->bUseJumpScareCamera = true;

    // Chuyển view của người chơi sang NPC (bây giờ NPC chính là ViewTarget)
    PC->SetViewTargetWithBlend(NPC, BlendTime, EViewTargetBlendFunction::VTBlend_Cubic);
    UE_LOG(LogTemp, Warning, TEXT("JumpScare: Chuyển view sang NPC (dùng JumpScareCamera)."));

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
    // Lấy PlayerController
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        // Chuyển view về Pawn của người chơi
        APawn* PlayerPawn = PC->GetPawn();
        if (PlayerPawn)
        {
            PC->SetViewTargetWithBlend(PlayerPawn, BlendTime, EViewTargetBlendFunction::VTBlend_Cubic);
            UE_LOG(LogTemp, Warning, TEXT("JumpScare: Trả lại camera cho nhân vật người chơi."));
        }
    }

    // Tắt cờ bUseJumpScareCamera trên NPC
    if (CachedOwnerComp)
    {
        if (AAIController* AICon = CachedOwnerComp->GetAIOwner())
        {
            if (ANPC* NPC = Cast<ANPC>(AICon->GetPawn()))
            {
                NPC->bUseJumpScareCamera = false;
            }
        }

        // Báo cho Behavior Tree biết task đã hoàn thành
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}

void UBTTask_JumpScare::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type Result)
{
    // Xóa timer nếu task bị hủy hoặc kết thúc sớm
    if (UWorld* World = OwnerComp.GetWorld())
    {
        World->GetTimerManager().ClearTimer(TimerHandle_JumpScare);
    }
    Super::OnTaskFinished(OwnerComp, NodeMemory, Result);
}
