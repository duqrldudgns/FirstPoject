// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Explosive.h"
#include "Thunderstroke.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API AThunderstroke : public AExplosive
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	AThunderstroke();

	/** Base shape collision */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Collision")
	class UCapsuleComponent* CapsuleCollision;

	void HitOverlap();

	FTimerHandle HitTimer;

	float HitDelay;
};
