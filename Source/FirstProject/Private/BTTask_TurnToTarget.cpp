// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_TurnToTarget.h"
#include "EnemyAIController.h"
#include "Enemy.h"
#include "Main.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"



UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
    NodeName = TEXT("Turn");
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    auto Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetPawn());
    if (nullptr == Enemy) return EBTNodeResult::Failed;
    
    auto Target = Cast<AMain>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AEnemyAIController::TargetKey));
    if (nullptr == Target) return EBTNodeResult::Failed;

    //FVector LookVector = Target->GetActorLocation() - Enemy->GetActorLocation();
    //LookVector.Z = 0.0f;
    //FRotator TargetRot = FRotationMatrix::MakeFromX(LookVector).Rotator();  //X 축 기준으로 얼마나 회전이 되었는지 계산
    FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(Enemy->GetActorLocation(), Target->GetActorLocation());
    TargetRot.Pitch = 0.f;
    TargetRot.Roll = 0.f;

    Enemy->SetActorRotation(FMath::RInterpTo(Enemy->GetActorRotation(), TargetRot, GetWorld()->GetDeltaSeconds(), 2.0f));

    return EBTNodeResult::Succeeded;
}