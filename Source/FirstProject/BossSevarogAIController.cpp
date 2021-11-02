// Fill out your copyright notice in the Description page of Project Settings.


#include "BossSevarogAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName ABossSevarogAIController::HomePosKey(TEXT("HomePos"));
const FName ABossSevarogAIController::PatrolPosKey(TEXT("PatrolPos"));
const FName ABossSevarogAIController::TargetKey(TEXT("Target"));
const FName ABossSevarogAIController::RandomNumKey(TEXT("RandomNum"));
const FName ABossSevarogAIController::DistanceKey(TEXT("Distance"));
const FName ABossSevarogAIController::TargetLocKey(TEXT("TargetLoc"));
const FName ABossSevarogAIController::CanAttackKey(TEXT("CanAttack"));
const FName ABossSevarogAIController::DamagedIngKey(TEXT("DamagedIng"));

ABossSevarogAIController::ABossSevarogAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBObject(TEXT("BlackboardData'/Game/Enemies/Sevarog/SevarogAIController/BB_Sevarog.BB_Sevarog'"));
	if (BBObject.Succeeded())
	{
		BBSevarog = BBObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("BehaviorTree'/Game/Enemies/Sevarog/SevarogAIController/BT_Sevarog.BT_Sevarog'"));
	if (BTObject.Succeeded())
	{
		BTSevarog = BTObject.Object;
	}

}

void ABossSevarogAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (UseBlackboard(BBSevarog, Blackboard))
	{
		RunBehaviorTree(BTSevarog);
		
		Blackboard->SetValueAsVector(HomePosKey, InPawn->GetActorLocation());
		Blackboard->SetValueAsInt(RandomNumKey, 2);
		Blackboard->SetValueAsFloat(DistanceKey, 10000.f);
		Blackboard->SetValueAsBool(CanAttackKey, true);
	}
}