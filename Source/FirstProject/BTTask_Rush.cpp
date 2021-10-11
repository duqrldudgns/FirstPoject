// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Rush.h"
#include "BossSevarogAIController.h"
#include "BossSevarog.h"
#include "Main.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Rush::UBTTask_Rush()
{
    NodeName = TEXT("Rush");
}

EBTNodeResult::Type UBTTask_Rush::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    auto Sevarog = Cast<ABossSevarog>(OwnerComp.GetAIOwner()->GetPawn());
    if (nullptr == Sevarog) return EBTNodeResult::Failed;

    auto Target = Cast<AMain>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABossSevarogAIController::TargetKey));
    if (nullptr == Target) return EBTNodeResult::Failed;


    return EBTNodeResult::Succeeded;
}