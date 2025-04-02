#include "HorrorGame/AI/BTTask_MeleeAttack.h"
#include "Combat_Interface.h"
#include "AIController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Engine/LatentActionManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "HorrorGame/HorrorGameCharacter.h" 
#include "NPC.h"

UBTTask_MeleeAttack::UBTTask_MeleeAttack()
{
    NodeName = TEXT("Melee Attack");
}

EBTNodeResult::Type UBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // Nếu mục tiêu ngoài tầm, kết thúc task ngay lập tức
    const bool bOutOfRange = !OwnerComp.GetBlackboardComponent()->GetValueAsBool(GetSelectedBlackboardKey());
    if (bOutOfRange)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return EBTNodeResult::Succeeded;
    }

    // Lấy AI Controller và NPC
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return EBTNodeResult::Failed;
    }

    ANPC* NPC = Cast<ANPC>(AICon->GetPawn());
    if (!NPC)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return EBTNodeResult::Failed;
    }

    // Lấy World để đặt Timer
    UWorld* World = NPC->GetWorld();
    if (!World)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return EBTNodeResult::Failed;
    }

    // Lưu trữ con trỏ của BehaviorTreeComponent và Task Node
    UBehaviorTreeComponent* OwnerCompPtr = &OwnerComp;
    UBTTask_MeleeAttack* NodePtr = this; // Con trỏ tới chính task này

    // Sử dụng Timer để thêm độ trễ 2 giây trước khi thực hiện tấn công
    FTimerHandle TimerHandle;
    FTimerDelegate TimerDelegate;

    TimerDelegate.BindLambda([NPC, OwnerCompPtr, NodePtr, World]()
        {
            // Kiểm tra an toàn: đảm bảo NPC, Mesh và AnimInstance hợp lệ
            if (!NPC || !NPC->GetMesh() || !NPC->GetMesh()->GetAnimInstance())
            {
                NodePtr->FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Failed);
                return;
            }

            // Lấy đối tượng người chơi và kiểm tra trạng thái sống
            AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
            AHorrorGameCharacter* PlayerCharacter = Cast<AHorrorGameCharacter>(PlayerActor);
            if (PlayerCharacter && PlayerCharacter->GetHealth() <= 0)
            {
                // Nếu người chơi đã chết, kết thúc task mà không thực hiện tấn công
                NodePtr->FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
                return;
            }

            UAnimInstance* AnimInstance = NPC->GetMesh()->GetAnimInstance();
            UAnimMontage* Montage = NPC->GetMontage();  // Giả sử ANPC có hàm GetMontage() trả về montage tấn công
            // Nếu Montage hợp lệ và đã kết thúc thì thực hiện tấn công
            if (Montage && AnimInstance->Montage_GetIsStopped(Montage))
            {
                if (ICombat_Interface* ICombat = Cast<ICombat_Interface>(NPC))
                {
                    ICombat->Execute_MeleeAttack(NPC);
                }
            }

            // Kết thúc task sau khi thực hiện hành động
            NodePtr->FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
        });

    // Đặt timer với độ trễ 2 giây (không lặp lại)
    World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 2.0f, false);

    // Task đang chờ Timer nên trả về InProgress
    return EBTNodeResult::InProgress;
}

bool UBTTask_MeleeAttack::MontageHasFinished(ANPC* const NPC)
{
    return NPC->GetMesh()->GetAnimInstance()->Montage_GetIsStopped(NPC->GetMontage());
}
