// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Arrow.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API AArrow : public AProjectile
{
	GENERATED_BODY()
	
public:

	AArrow();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	class ABow* BowReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float DelayBeforeGravity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float RadiusCheck;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	void RotationUpdate(FRotator NewRotation);

	UFUNCTION(BlueprintCallable)
	void OnOtherHit(AActor* Other, FVector HitLocation);
};
