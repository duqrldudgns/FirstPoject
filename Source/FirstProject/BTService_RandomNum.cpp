// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_RandomNum.h"
#include "BossSevarogAIController.h"
#include "Enemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Math/UnrealMathUtility.h"

UBTService_RandomNum::UBTService_RandomNum()
{
	NodeName = TEXT("RandomNum");
	Interval = 1.0f;

	bCallTickOnSearchStart = true;	//검색 시작 시 틱 이벤트 호출
}

void UBTService_RandomNum::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AEnemy* ControllingPawn = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (nullptr == ControllingPawn) return;

	int32 Num = FMath::RandRange(0,1);
	OwnerComp.GetBlackboardComponent()->SetValueAsInt(ABossSevarogAIController::RandomNumKey, Num);
}
