// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack2.h"
#include "EnemyAIController.h"
#include "BossSevarog.h"

UBTTask_Attack2::UBTTask_Attack2()
{
    bNotifyTick = true; //틱 기능 활성화
    IsAttacking = false;

}

EBTNodeResult::Type UBTTask_Attack2::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    auto BossSevarog = Cast<ABossSevarog>(OwnerComp.GetAIOwner()->GetPawn());
    if (nullptr == BossSevarog) return EBTNodeResult::Failed;

    BossSevarog->RushAttack();
    IsAttacking = true;

    BossSevarog->OnAttackEnd.AddLambda([this]() ->void {IsAttacking = false; });

    return EBTNodeResult::InProgress;      // 공격 애니메이션이 끝날 떄까지 대기해야 하는 지연 태스크이므로 결과 값을 InProgress로 반환하고, 공격이 끝났을 떄 태스크가 끝났다고 알려줘야 한다. 이를 알려주는 함수가 FinistLatentTask 이다.
}


void UBTTask_Attack2::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
    if (!IsAttacking)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);   // 이 함수를 호출하지 않으면 행동트리 시스템은 현재 태스크에 계속 머물어 있는다. 그래서 호출 할수 있도록 노드의 Tick 기능을 활성화하고 조건을 파악한 후 태스크 종료 명령을 내려줘야 한다
    }
}