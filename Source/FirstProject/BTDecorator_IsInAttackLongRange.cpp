// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_IsInAttackLongRange.h"
#include "EnemyAIController.h"
#include "Main.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsInAttackLongRange::UBTDecorator_IsInAttackLongRange()
{
	NodeName = TEXT("CanLongAttack");

	Dist = 1000.f;
}

bool UBTDecorator_IsInAttackLongRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn) return false;

	AMain* Target = Cast<AMain>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AEnemyAIController::TargetKey));
	if (nullptr == Target) return false;

	bResult = (Target->GetDistanceTo(ControllingPawn) >= Dist);	// Target���� �Ÿ��� �����Ѹ�ŭ �Ǹ� true ��ȯ �Ͽ� ����
	return bResult;
}
