// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Rush.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API UBTTask_Rush : public UBTTaskNode
{
	GENERATED_BODY()
	
public:

	UBTTask_Rush();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
