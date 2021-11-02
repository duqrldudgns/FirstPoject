// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "BossSevarog.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API ABossSevarog : public AEnemy
{
	GENERATED_BODY()
	
public:
	ABossSevarog();

	virtual void BeginPlay() override;

	virtual void AttackEnd() override;

	UFUNCTION()
	virtual void CapsuleOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	class ABossSevarogAIController* Controller;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	class UParticleSystemComponent* AuraParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims")
	class UAnimMontage* RushAttackMontage;

	void RushAttack();

	UFUNCTION(BlueprintCallable)
	void Rush();


	UFUNCTION(BlueprintCallable)
	void ThunderstrokeAttack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims")
	class UAnimMontage* ThunderstrokeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AThunderstroke> Thunderstroke;

	UFUNCTION(BlueprintCallable)
	void CreateThunderDecal();

	UFUNCTION(BlueprintCallable)
	void ActiveThunderstroke();

	FVector ThunderSpawnLoc;

	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	TSubclassOf<class APositionDecal> PositionDecal;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	FVector GetRandomPoint();

	TArray<FVector> RandomPoints;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

};
