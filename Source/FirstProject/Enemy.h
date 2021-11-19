// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyMovementStatus : uint8
{
	EMS_Idle			UMETA(DisplayName = "Idle"),
	EMS_MoveToTarget	UMETA(DisplayName = "MoveToTarget"),
	EMS_Attacking		UMETA(DisplayName = "Attacking"),
	EMS_Dead			UMETA(DisplayName = "Dead"),

	EMS_MAX				UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class FIRSTPROJECT_API AEnemy : public ACharacter
{
	GENERATED_BODY()

	/** 플레이어의 공격이 종료되면 공격 태스크에서 해당 알림을 받을 수 있도록 델리게이트를 새로 선언 */
	DECLARE_MULTICAST_DELEGATE(FOnAttackEndDelegate);

public:
	// Sets default values for this character's properties
	AEnemy();

	/** 공격이 종료될 떄 델리게이트를 호출 */
	FOnAttackEndDelegate OnAttackEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EEnemyMovementStatus EnemyMovementStatus;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetEnemyMovementStatus(EEnemyMovementStatus Status);
	FORCEINLINE EEnemyMovementStatus GetEnemyMovementStatus() { return EnemyMovementStatus; }
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	//class USphereComponent* AgroSphere;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	//class USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Decal")
	class UDecalComponent* SelectDecal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* HealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sword", meta = (AllowPrivateAccess = "true"))
	class UChildActorComponent* SwordAttached;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIController* AIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* SwingSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* CombatCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims")
	class UAnimMontage* CombatMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims")
	UAnimMontage* DamagedMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims")
	UAnimMontage* LightDamagedMontage;

	// Enemy마다 타격 시 생성하려는 파티클 유형(피 분출 등)이 다르기 때문에 Enemy에 파티클시스템을 달고있음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UParticleSystem* HitParticles;

	FTimerHandle AttackTimer;

	
	/** 손상 유형 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//UFUNCTION()
	//virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//UFUNCTION()
	//virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//UFUNCTION()
	//virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//UFUNCTION()
	//virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void MoveToTarget(class AMain* Target);

	/** 블루프린트 브런치에 사용 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bOverlappingCombatSphere;	

	/** Anim_BP에서 Target(Main)에 대한 정보를 전달 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	AMain* CombatTarget;	
	
	/** 대상의 생존 여부에 따라 공격 가능 구분 */
	bool bHasValidTarget;

	FTimerHandle TimerHandle;
	float TimerRemaining;

	int32 DamType;
	UFUNCTION(BlueprintCallable)
	void UpperActivate();
	UFUNCTION(BlueprintCallable)
	void KnockDownActivate();
	UFUNCTION(BlueprintCallable)
	void RushActivate();
	UFUNCTION(BlueprintCallable)
	void ActivateCollision();
	UFUNCTION(BlueprintCallable)
	void DeActivateCollision();

	/** Attack */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bAttacking;

	void Attack();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();


	/** ApplyDamage 함수와 연동되어 데미지를 입으면 실행 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	bool Alive();

	void Die();

	bool DecrementHealth(float Amount);

	void Disappear();
	
	UFUNCTION(BlueprintCallable)
	void AirHitEnd();

	UFUNCTION(BlueprintCallable)
	void DamagedDownEnd();

	UFUNCTION(BlueprintCallable)
	void DamagedEnd();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bDamagedIng;

	
	float DetectRadius;

	void PlayMontage(UAnimMontage* MontageName);

	class UAnimInstance* AnimInstance;

	
	FTimerHandle DisplayHealthBarHandle;
	float DisplayHealthBarTime;
	
	void DisplayHealthBar();
	void RemoveHealthBar();

	/** 한번 맞을 때 중복해서 안맞도록 */
	bool bHitOnce;

	void ResetHitOnce();

	void ResetCasting();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bCanAttack;

	void AttackDelay();
	float AttDelay;

	void StopAnim();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bFreezing;

	bool bDie;
};
