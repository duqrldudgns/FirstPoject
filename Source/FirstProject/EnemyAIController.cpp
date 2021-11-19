// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName AEnemyAIController::HomePosKey(TEXT("HomePos"));
const FName AEnemyAIController::PatrolPosKey(TEXT("PatrolPos"));
const FName AEnemyAIController::TargetKey(TEXT("Target"));
const FName AEnemyAIController::RandomNumKey(TEXT("RandomNum"));
const FName AEnemyAIController::CanAttackKey(TEXT("CanAttack"));
const FName AEnemyAIController::DamagedIngKey(TEXT("DamagedIng"));
const FName AEnemyAIController::FreezingKey(TEXT("Freezing"));
const FName AEnemyAIController::DieKey(TEXT("Die"));

AEnemyAIController::AEnemyAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBObject(TEXT("BlackboardData'/Game/Enemies/AIController/BB_Enemy.BB_Enemy'"));
	if (BBObject.Succeeded())
	{
		BBEnemy = BBObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("BehaviorTree'/Game/Enemies/AIController/BT_Enemy.BT_Enemy'"));
	if (BTObject.Succeeded())
	{
		BTEnemy = BTObject.Object;
	}

}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (UseBlackboard(BBEnemy, Blackboard))
	{
		RunBehaviorTree(BTEnemy);

		Blackboard->SetValueAsVector(HomePosKey, InPawn->GetActorLocation());
		Blackboard->SetValueAsInt(RandomNumKey, 2);
		Blackboard->SetValueAsBool(CanAttackKey, true);
	}
}