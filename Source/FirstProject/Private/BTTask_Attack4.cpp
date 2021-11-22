// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack4.h"
#include "EnemyAIController.h"
#include "BossSevarog.h"

UBTTask_Attack4::UBTTask_Attack4()
{
    bNotifyTick = true; //틱 기능 활성화
    IsAttacking = false;

}

EBTNodeResult::Type UBTTask_Attack4::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    auto BossSevarog = Cast<ABossSevarog>(OwnerComp.GetAIOwner()->GetPawn());
    if (nullptr == BossSevarog) return EBTNodeResult::Failed;

    BossSevarog->SpawnEnemyAttack();
    IsAttacking = true;

    BossSevarog->OnAttackEnd.AddLambda([this]() ->void {IsAttacking = false; });

    return EBTNodeResult::InProgress;      
}


void UBTTask_Attack4::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
    if (!IsAttacking)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
