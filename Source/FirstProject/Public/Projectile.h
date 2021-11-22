// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Projectile.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API AProjectile : public AItem
{
	GENERATED_BODY()
	
public:

	AProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UArrowComponent* CheckImpactLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	AController* ProjectileInstigator;

	FORCEINLINE void SetInstigator(AController* Inst) { ProjectileInstigator = Inst; }
};
