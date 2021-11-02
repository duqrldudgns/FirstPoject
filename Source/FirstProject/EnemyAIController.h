// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:

	AEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;

	static const FName HomePosKey;
	static const FName PatrolPosKey;
	static const FName TargetKey;
	static const FName RandomNumKey;
	static const FName CanAttackKey;
	static const FName DamagedIngKey;
private:

	UPROPERTY()
	class UBlackboardData* BBEnemy;

	UPROPERTY()
	class UBehaviorTree* BTEnemy;

};
