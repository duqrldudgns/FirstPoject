// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossSevarogAIController.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API ABossSevarogAIController : public AAIController
{
	GENERATED_BODY()

public:

	ABossSevarogAIController();

	virtual void OnPossess(APawn* InPawn) override;

	static const FName HomePosKey;
	static const FName PatrolPosKey;
	static const FName TargetKey;
	static const FName RandomNumKey;
	static const FName DistanceKey;
	static const FName TargetLocKey;
	static const FName CanAttackKey;
	static const FName DamagedIngKey;
	static const FName FreezingKey;
	static const FName DieKey;

private:

	UPROPERTY()
		class UBlackboardData* BBSevarog;

	UPROPERTY()
		class UBehaviorTree* BTSevarog;

};
