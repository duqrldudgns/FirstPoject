// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_AttackDelay.h"
#include "BossSevarogAIController.h"
#include "EnemyAIController.h"
#include "Enemy.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_AttackDelay::UBTService_AttackDelay()
{
	NodeName = TEXT("AttackDelay");
	Interval = 0.5f;
}

void UBTService_AttackDelay::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AEnemy* ControllingPawn = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (nullptr == ControllingPawn) return;

	OwnerComp.GetBlackboardComponent()->SetValueAsBool(ABossSevarogAIController::CanAttackKey, ControllingPawn->bCanAttack);

	OwnerComp.GetBlackboardComponent()->SetValueAsBool(ABossSevarogAIController::DamagedIngKey, ControllingPawn->bDamagedIng);
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(ABossSevarogAIController::FreezingKey, ControllingPawn->bFreezing);
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(AEnemyAIController::FreezingKey, ControllingPawn->bFreezing);
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(ABossSevarogAIController::DieKey, ControllingPawn->bDie);
	OwnerComp.GetBlackboardComponent()->SetValueAsBool(AEnemyAIController::DieKey, ControllingPawn->bDie);
}
