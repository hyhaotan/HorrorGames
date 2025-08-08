// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ChasePlayer.h"
#include "HorrorGame/AI/NPC_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "HorrorGame/AI/NPC.h" 

UBTTask_ChasePlayer::UBTTask_ChasePlayer(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = TEXT("Chase Player");
}

EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (auto* const cont = Cast<ANPC_AIController>(OwnerComp.GetAIOwner()))
    {
        auto const PlayerLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(GetSelectedBlackboardKey());

        // Movement to the player
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(cont, PlayerLocation);

        auto* const NPC = Cast<ANPC>(cont->GetPawn());
        auto const BB = OwnerComp.GetBlackboardComponent();

        if (BB && NPC)
        {
            bool bCanSeePlayer = BB->GetValueAsBool(TEXT("CanSeePlayer"));
            NPC->ToggleInvestigationWidgetVisibility(bCanSeePlayer);
        }

        // Finish success
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return EBTNodeResult::Succeeded;
    }
    return EBTNodeResult::Failed;
}
