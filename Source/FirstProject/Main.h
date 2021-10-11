// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	//EMS : E Movement Status
	EMS_Normal		UMETA(DisplayName = "Normal"),		//기본 달리기
	EMS_Sprinting	UMETA(DisplayName = "Sprinting"),	//전력질주
	EMS_Guard		UMETA(DisplayName = "Guard"),		//가드
	EMS_Dead		UMETA(DisplayName = "Dead"),

	EMS_MAX			UMETA(DisplayName = "DefaultMAX")	//사용하기 위한 것은 아니지만 이를 제한하고 명확한 최댓값을 나타냄
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	//ESS : E Stamina Status
	ESS_Normal				UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum		UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted			UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX					UMETA(DisplayName = "DefaultMAX")
};

//UENUM(BlueprintType)
//enum class EArmedStatus : uint8
//{
//	//EAS : E Armed Status
//	EAS_Normal				UMETA(DisplayName = "Normal"),
//	EAS_Sword				UMETA(DisplayName = "Sword"),
//
//	ESS_MAX					UMETA(DisplayName = "DefaultMAX")
//};

UCLASS()
class FIRSTPROJECT_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	/** Debug */
	TArray<FVector> PickupLocations;

	UFUNCTION(BlueprintCallable)
	void ShowPickupLocations();

	///** Armed Status*/
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	//EArmedStatus ArmedStatus;

	//void SetArmedStatus(EArmedStatus Status);
	//FORCEINLINE EArmedStatus GetArmedStatus() { return ArmedStatus; }

	/** Stamina Status */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MinSprintStamina;


	/**
	* 보간(Interpolation) - 두 점을 연결하는 방법
	* 적이 나를 공격대상으로 정했을 때 그 대상을 향하도록 설정 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	FRotator GetLookAtRotationYaw(FVector Target);

	float InterpSpeed;
	bool bInterpToEnemy;
	void SetInterpToEnemy(bool Interp);

	void UpdateCombatTarget();

	// AEnemy class만 수집
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AEnemy> EnemyFilter;

	// 내가 적의 공격 대상으로 설정되어 있다면 그 적의 HP바가 보임
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;


	/** Set movement status and running speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;
	
	void SetMovementStatus(EMovementStatus Status);
	FORCEINLINE EMovementStatus GetMovementStatus() { return MovementStatus; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float WalkingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintingSpeed;

	bool bShiftKeyDown;

	/** Pressed down to enable sprinting */
	void ShiftKeyDown();

	/** Released to stop sprinting */
	void ShiftKeyUp();

	virtual void Jump() override;

	/** Camera boom positioning the camera behind the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))	// 코드상에서는 private이지만 에디터 상에서는 수정 가능
	class USpringArmComponent* CameraBoom;

	/** Follow Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rates to scale turning functions for the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Decal")
	class UDecalComponent* SelectDecal;


	/** Foot IK */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	class UCpt_FootIK* FootIK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	FRotator FootRotationL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	FRotator FootRotationR;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	float HipOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	float FootOffsetL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	float FootOffsetR;
	



	/**
	/*
	/* Player Stats
	/*
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int32 Coins;

	/** ApplyDamage 함수와 연동되어 데미지를 입으면 실행 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void IncrementCoins(int32 Amount);
	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	void DecrementHealth(float Amount);
	void DecrementHealth(float Amount, AActor* DamageCauser);

	bool Alive();

	void Die();


	// 이 캐릭터는 맞을 때 이 피를 분출함
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound;
			
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for Yaw rotation */
	void Turn(float Value);

	/** Called for Pitch rotation */
	void LookUp(float Value);

	bool bMovingForward;
	bool bMovingRight;

	bool CanMove(float Value);

	bool CanAction();

	/** Called for forwards/backwards input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Called via input to turn at a given rate
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/** Called via input to look up/down at a given rate
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired look up/down rate
	*/
	void LookUpAtRate(float Rate);

	bool bLMBDown;
	void LMBDown();
	void LMBUp();

	bool bRMBDown;
	void RMBDown();
	void RMBUp();

	bool bEscDown;
	void EscDown();
	UFUNCTION(BlueprintCallable)
	void EscUp();

	bool bFirstSkillKeyDown;
	void FirstSkillKeyDown();
	void FirstSkillKeyUp();

	bool bSecondSkillKeyDown;
	void SecondSkillKeyDown();
	void SecondSkillKeyUp();

	bool bThirdSkillKeyDown;
	void ThirdSkillKeyDown();
	void ThirdSkillKeyUp();

	bool bFourthSkillKeyDown;
	void FourthSkillKeyDown();
	void FourthSkillKeyUp();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Test")
	bool bTestKeyDown;
	void TestKeyDown();
	void TestKeyUp();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


	/** Equip pick up*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AItem* ActiveOverlappingItem;		// AItem으로 생성한 이유는 아이템의 종류가 갑옷이든 소모품일 수 있기 때문에 모두 포함하기 위함

	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }
	
	bool bInteractionKeyDown;
	void InteractionKeyDown();
	void InteractionKeyUp();


	/** Equip - Weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	void SetEquippedWeapon(AWeapon* WeaponToSet);
	FORCEINLINE AWeapon* GetEquippedWeapon() { return EquippedWeapon; }

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Anims")
	bool bArmedBridge;	// 브릿지 애니메이션을 실행 했는지
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Anims")
	bool bArmedBridgeIng;

	UFUNCTION(BlueprintCallable)
	void ArmedBridgeStart();

	UFUNCTION(BlueprintCallable)
	void ArmedBridgeEnd();


	/** Attack */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Anims")
	bool bAttacking;

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	int ComboCnt;
	bool bComboAttackInput;

	UFUNCTION(BlueprintCallable)
	void ComboAttackInputCheck();

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	UFUNCTION(BlueprintCallable)
	void Resurrection();


	/** Jump Combat */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* JumpCombatMontage;

	void AirAttack();


	/** Defense */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* DamagedMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* GuardAcceptSound;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	bool bGuardAccept;

	FORCEINLINE void SetGuardAccept(bool Accept) { bGuardAccept = Accept; }
	FORCEINLINE bool GetGuardAccept() { return bGuardAccept; }

	UFUNCTION(BlueprintCallable)
	void GuardAcceptEnd();

	/** Save & Load Data*/
	void SwitchLevel(FName LevelName);

	UFUNCTION(BlueprintCallable)
	void SaveGame();
	
	UFUNCTION(BlueprintCallable)
	void LoadGame(bool SetPosition);

	/** 레벨을 시작할 때 기본적으로 로드되는 것 */
	void LoadGameNoSwitch();

	UPROPERTY(EditDefaultsOnly, Category = "SaveData")
	TSubclassOf<class AItemStorage> WeaponStorage;


	/** Skill */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Anims")
	bool bSkillCasting;

	UFUNCTION(BlueprintCallable)
	void SkillCastEnd();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* SkillMontage;


	/** 피해를 입힐 때 전달해야 하는 컨트롤러가 필요 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	AController* MainInstigator;

	/** 피해를 입힐 때 메인컨트롤러를 얻어야함 */
	FORCEINLINE void SetInstigator(AController* Inst) { MainInstigator = Inst; }

	void PlaySkillMontage(FName Section);

	/** FireBallSkill */
	void FireBallSkillCast();

	UFUNCTION(BlueprintCallable)
	void FireBallSkillActivation();

	UFUNCTION(BlueprintCallable)
	void PlayFireBallSkillParticle();

	UFUNCTION(BlueprintCallable)
	void PlayFireBallSkillSound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* FireBallSkillParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* FireBallSkillSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AShootingSkill> FireBall;


	/** WaveSkill */
	void WaveSkillCast();

	UFUNCTION(BlueprintCallable)
	void WaveSkillActivation();

	UFUNCTION(BlueprintCallable)
	void PlayWaveSkillParticle();

	UFUNCTION(BlueprintCallable)
	void PlayWaveSkillSound();

	UFUNCTION(BlueprintCallable)
	void SkillKeyDownCheck();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* WaveSkillParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* WaveSkillSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float WaveSkillDamage;

	/** Dodge */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* DodgeMontage;

	/** Dash Attack */
	void DashAttack();

	/** Upper Attack */
	void UpperAttack();


	/** DamageTypeClass */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> Basic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> Upper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> KnockDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> Rush;


	FTimerHandle TimerHandle;


	FVector GetFloor();
};
