// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bow.generated.h"

UENUM(BlueprintType)
enum class EArrowStatus : uint8
{
	//EAS : E Arrow Status
	EAS_Normal				UMETA(DisplayName = "Normal"),
	EAS_Rain				UMETA(DisplayName = "Rain"),
	EAS_Ice					UMETA(DisplayName = "Ice"),
	EAS_Detection			UMETA(DisplayName = "Detection"),

	ESS_MAX					UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class FIRSTPROJECT_API ABow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABow();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	class UPostProcessComponent* PostProcessEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	USkeletalMeshComponent* Bow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	class UCableComponent* TopCable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	class UCableComponent* BottomCable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	class UParticleSystemComponent* RainOfArrowsEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	class UAudioComponent* SkillEffectSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	UParticleSystemComponent* IceArrowEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	UParticleSystemComponent* DetectionArrowEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	USceneComponent* ArrowScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	UStaticMeshComponent* ArrowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	USceneComponent* ArrowSpawnLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	UPostProcessComponent* PostProcessMaterials;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	UAudioComponent* DrawSound;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Arrow Status*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EArrowStatus ArrowStatus;

	FORCEINLINE void SetArrowStatus(EArrowStatus Status) { ArrowStatus = Status; }
	FORCEINLINE EArrowStatus GetArrowStatus() { return ArrowStatus; }

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<class AArrow> Normal;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AArrow> Rain;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AArrow> Ice;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AArrow> Detection;

	UPROPERTY(EditDefaultsOnly, Category = "Bow")
	TSubclassOf<UDamageType> RainType;

	UPROPERTY(EditDefaultsOnly, Category = "Bow")
	TSubclassOf<UDamageType> IceType;

	UPROPERTY(EditDefaultsOnly, Category = "Bow")
	TSubclassOf<UDamageType> DetectionType;

	/** Check to activate only one skill */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool bArrowSkillActive;

	/** Skill - Rain Of Arrows */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float RainOfArrowsDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float RainOfArrowsCoolDownTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float RainOfArrowsCoolDownUI;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool RainOfArrowsCoolDown;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool RainOfArrowsActive;

	void SetRainOfArrowsActive(bool Condition);
	void StartRainOfArrows();
	void StopRainOfArrows();
	void StartRainOfArrowsCooldown();
	void RainOfArrows(FHitResult Hit);

	UFUNCTION(BlueprintNativeEvent)
	void RainOfArrowsEffectActivate();
	UFUNCTION(BlueprintNativeEvent)
	void RainOfArrowsEffectDeactivate();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetUIRainOfArrowsCoolDown();
	UFUNCTION(BlueprintNativeEvent)
	void RainOfArrowsHit(FVector StartLoc, FVector Location);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	class UParticleSystem* RainOfArrowsBurst;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	UParticleSystem* RainOfArrowsParticle;

	/** Skill - IceArrow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float IceArrowCoolDownTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float IceArrowCoolDownUI;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool IceArrowCoolDown;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool IceArrowActive;

	void SetIceArrowActive(bool Condition);
	void StartIceArrow();
	void StopIceArrow();
	void StartIceArrowCooldown();
	void IceArrow(FHitResult Hit);

	UFUNCTION(BlueprintNativeEvent)
	void IceArrowEffectActivate();
	UFUNCTION(BlueprintNativeEvent)
	void IceArrowEffectDeactivate();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetUIIceArrowCoolDown();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	TArray<UMaterialInterface*> OriginalMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	UMaterialInterface* IceMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	UParticleSystem* IceArrowParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	class USoundCue* IceArrowSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	class AEnemy* IceTarget;

	/** Skill - DetectionArrow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float DetectionTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float DetectionArrowCoolDownTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float DetectionArrowCoolDownUI;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool DetectionArrowCoolDown;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool DetectionArrowActive;

	void SetDetectionArrowActive(bool Condition);
	void StartDetectionArrow();
	void StopDetectionArrow();
	void StartDetectionArrowCooldown();
	void DetectionArrow(FVector HitLoc);

	UFUNCTION(BlueprintNativeEvent)
	void DetectionArrowEffectActivate();
	UFUNCTION(BlueprintNativeEvent)
	void DetectionArrowEffectDeactivate();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetUIDetectionArrowCoolDown();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	UParticleSystem* DetectionArrowParticle;

	TArray<FHitResult> DetectionHitResults;

	/** getting some of the player variables */
	void BeginPlayBow(ACharacter* Object);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	class AMain* CharReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float CharacterDefaultFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float CharacterDefaultCameraArmLength;
	

	/** Arrows ammo & Aim & cooldown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "widgets")
	TSubclassOf<UUserWidget> WUIWidgetReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "widgets")
	UUserWidget* UIWidgetReference;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UpdateCrossHairWithSpread(float CurrentSpread);

	UFUNCTION(BlueprintNativeEvent)
	void UpdateAmmoUI(int32 Arrows);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float CircleScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float AimAssistPlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	int32 MaxArrows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	FText AmmoText;


	/** starting a trace for the lcoation and adding the UI to the screen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "widgets")
	TSubclassOf<UUserWidget> WCantHitWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "widgets")
	UUserWidget* CantHitWidget;


	/** Static mesh obstacles in front of the player? */
	void CheckIfCanFire();

	/** Identify what the player is looking at. */
	FVector CameraTrace();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool IsAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float ArrowStartLocationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float ObstacleMaxDistanceDivider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float TraceSpreadMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float TraceSpreadMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float TraceSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float TraceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	AActor* HitActorObstacleCheck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	FVector CantHitWidgetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool ShowCantHitWidget;


	/** Show and Hide arrow & cable during shooting. */
	void HideArrow();
	
	void ShowArrow();

	void HoldCable();
	
	void DontHoldCable();	//잡아당기고 있지 않음


	/** Bow Equip & UnEquip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	USoundCue* BowEquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	USoundCue* BowUnEquipSound;

	void EquippedChanged(bool NewHidden);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool ArrowInvisibility;

	void UpdateArrowLocationRotation();

	bool DoOnce;

	/** Aim */
	/** Replication events for aiming */
	void AimStart();
	void AimStop();

	/** All the local changes when staritng and stopping aiming */
	void AimChange(bool Aiming);

	/** Hide or Show cant hit ui */
	UFUNCTION(BlueprintNativeEvent)
	void HideOrShow(bool Hide);

	/** If you are Aiming, Post process fade in/out */ 
	UFUNCTION(BlueprintNativeEvent)
	void FadeInPostProcess(float PlayRatePostProcess);
	UFUNCTION(BlueprintNativeEvent)
	void FadeOutPostProcess(float PlayRatePostProcess);

	/** Arm Length Change */
	UFUNCTION(BlueprintNativeEvent)
	void CameraArmLengthChanges(float SubstractionArmLength);
	UFUNCTION(BlueprintNativeEvent)
	void CameraArmLengthChangesRevert();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float AimSubstractionArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float TargetArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float CurrentArmLength;

	/** Crosshair Fade in / out */
	UFUNCTION(BlueprintNativeEvent)
	void FadeInCrosshair();
	UFUNCTION(BlueprintNativeEvent)
	void FadeOutCrosshair();

	/** Slow Motion */
	UFUNCTION(BlueprintNativeEvent)
	void SlowMotion();
	UFUNCTION(BlueprintNativeEvent)
	void StopSlowMotion();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float StartAimSlowMotionPlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float CurrentTimeDilation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float SlowMotionDilation;


	/** Is player trying to draw */
	void DrawStart();
	void DrawStop();
	void DrawingChanges();
	void StopDraw();
	void InteruptedDrawShoot();
	void DrawShoot();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool TryingToDraw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	bool IsDrawing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float DrawChangesPlayRate;

	/** Firing the projectile */
	void Shoot();

	bool CanShootAndSubtractAmmo();

	FTransform CalculateProjectileInfo();

	void SpawnProjectile(FTransform SpawnTransform);

	/** Fire Effects & sound */
	void FireEffects();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	class USoundCue* ShootSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	UParticleSystem* FireParticle;

	/** Field of view changes */
	UFUNCTION(BlueprintNativeEvent)
	void ChangeFOV(float FOVToSubtract, float FOVChangesPlayRate);
	UFUNCTION(BlueprintNativeEvent)
	void RestoreFOV(float FOVChangesPlayRate);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float SubtractionFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float ShootCameraChangesPlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float TargetFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float CurrentFOV;

	/** Damage settings timeline */
	UFUNCTION(BlueprintNativeEvent)
	void StartDamageNumbersTimeline(float DamageSettingsPlayRate);
	UFUNCTION(BlueprintNativeEvent)
	void StopDamageNumbersTimeline();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float DamageMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float DamageMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float ShootingDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float ShootingDistanceMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float ShootingDistanceMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float ProjectileSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float ProjectileSpeedMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	float ProjectileSpeedMax;


	/** Apply Damage */
	void DamageEvent(FHitResult HitResult, float BaseDamage, TSubclassOf<UDamageType> ArrowDamageType);

	void SpawnArrowPickUp(FHitResult HitResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	TSubclassOf<class AArrowPickUp> ArrowPickUpAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	AArrowPickUp* ArrowPickUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	UMaterialInterface* BurningDecal;
	

	/** Play Particle & Sound At Location */
	void PlayParticleAtLocation(FHitResult Hit);
	void PlaySoundAtLocation(FHitResult Hit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Particles")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Particles")
	UParticleSystem* BrickImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Particles")
	UParticleSystem* DirtImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Particles")
	UParticleSystem* WoodImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Particles")
	UParticleSystem* BloodImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Particles")
	UParticleSystem* WaterImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Particles")
	UParticleSystem* GrassImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Sounds")
	USoundCue* DefaultImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Sounds")
	USoundCue* BrickImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Sounds")
	USoundCue* DirtImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Sounds")
	USoundCue* WoodImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Sounds")
	USoundCue* BloodImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Sounds")
	USoundCue* WaterImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | ImPact_Sounds")
	USoundCue* GrassImpactSound;

};