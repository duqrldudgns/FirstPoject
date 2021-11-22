// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_DistToTarget.h"
#include "BossSevarogAIController.h"
#include "Enemy.h"
#include "Main.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_DistToTarget::UBTService_DistToTarget()
{
	NodeName = TEXT("DistanceToTarget");
	Interval = 0.5f;
}

void UBTService_DistToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AEnemy* ControllingPawn = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (nullptr == ControllingPawn) return;

	AMain* Target = Cast<AMain>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABossSevarogAIController::TargetKey));
	if (nullptr == Target) return;

	UE_LOG(LogTemp, Warning, TEXT("%f"), Target->GetDistanceTo(ControllingPawn));

	OwnerComp.GetBlackboardComponent()->SetValueAsFloat(ABossSevarogAIController::DistanceKey, Target->GetDistanceTo(ControllingPawn));
}
