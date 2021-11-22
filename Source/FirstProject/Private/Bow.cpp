// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"
#include "CableComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/PostProcessComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Blueprint/UserWidget.h"
#include "MainPlayerController.h"
#include "Sound/SoundCue.h"
#include "ArrowPickUp.h"
#include "Arrow.h"
#include "Enemy.h"
#include "Main.h"

// Sets default values
ABow::ABow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	PostProcessEffect = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessEffect"));
	PostProcessEffect->SetupAttachment(GetRootComponent());
	
	Bow = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Bow"));
	Bow->SetupAttachment(GetRootComponent());
	Bow->SetRelativeScale3D(FVector(1.8f, 1.8f, 1.3f));
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("SkeletalMesh'/Game/ArcherySystem/Bow/Meshes/Bow_1/SK_Bow_1.SK_Bow_1'"));
	if (MeshAsset.Succeeded())
	{
		Bow->SetSkeletalMesh(MeshAsset.Object);
	}

	TopCable = CreateDefaultSubobject<UCableComponent>(TEXT("TopCable"));
	TopCable->SetupAttachment(Bow, "TopWire");

	BottomCable = CreateDefaultSubobject<UCableComponent>(TEXT("BottomCable"));
	BottomCable->SetupAttachment(Bow, "BottomWire");
	
	RainOfArrowsEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RainOfArrowsEffect"));
	RainOfArrowsEffect->SetupAttachment(Bow);
	RainOfArrowsEffect->SetAutoActivate(false);

	SkillEffectSound = CreateDefaultSubobject<UAudioComponent>(TEXT("SkillEffectSound"));
	SkillEffectSound->SetupAttachment(RainOfArrowsEffect);
	SkillEffectSound->SetAutoActivate(false);
	SkillEffectSound->bOverrideAttenuation = true;

	IceArrowEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("IceArrowEffect"));
	IceArrowEffect->SetupAttachment(Bow);
	IceArrowEffect->SetAutoActivate(false);

	DetectionArrowEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DetectionArrowEffect"));
	DetectionArrowEffect->SetupAttachment(Bow);
	DetectionArrowEffect->SetAutoActivate(false);

	ArrowScene = CreateDefaultSubobject<USceneComponent>(TEXT("ArrowScene"));
	ArrowScene->SetupAttachment(GetRootComponent());

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(ArrowScene);
	ArrowMesh->SetCollisionProfileName("NoCollision");

	ArrowSpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ArrowSpawnLocation"));
	ArrowSpawnLocation->SetupAttachment(GetRootComponent());

	PostProcessMaterials = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessMaterials"));
	PostProcessMaterials->SetupAttachment(GetRootComponent());
	PostProcessMaterials->BlendWeight = 0.f;

	DrawSound = CreateDefaultSubobject<UAudioComponent>(TEXT("DrawSound"));
	DrawSound->SetupAttachment(GetRootComponent());
	DrawSound->SetAutoActivate(false);

	ArrowStartLocationOffset = 10.f;
	ObstacleMaxDistanceDivider = 4.f;
	TraceRadius = 5.f;
	AimAssistPlayRate = 1.f;
	MaxArrows = 11;
	AimSubstractionArmLength = 100.f;
	StartAimSlowMotionPlayRate = 0.25f;
	SlowMotionDilation = 0.25f;
	SubtractionFOV = 20.f;
	DrawChangesPlayRate = 1.f;
	ShootCameraChangesPlayRate = 10.f;
	DamageMin = 10.f;
	DamageMax = 100.f;
	TraceSpreadMax = 3.f;
	ShootingDistanceMin = 2500.f;
	ShootingDistanceMax = 4000.f;
	ProjectileSpeed = 1000.f;
	ProjectileSpeedMin = 2000.f;
	ProjectileSpeedMax = 5000.f;
	RainOfArrowsCoolDownTime = 10.f;
	RainOfArrowsDamage = 15.f;
	IceArrowCoolDownTime = 7.f;
	DetectionArrowCoolDownTime = 15.f;
	DetectionTime = 5.f;
	RainOfArrowsCoolDownUI = 1.f;
	IceArrowCoolDownUI = 1.f;
	DetectionArrowCoolDownUI = 1.f;
}

// Called when the game starts or when spawned
void ABow::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckIfCanFire();

	UpdateArrowLocationRotation();
}

void ABow::BeginPlayBow(ACharacter* Object)
{
	// Set Basic
	CharReference = Cast<AMain>(Object);
	if (nullptr == CharReference) return;
	CharacterDefaultFOV = CharReference->DefaultFieldOfView;
	CharacterDefaultCameraArmLength = CharReference->DefaultArmLength;

	// Set Arrows ammo & Aim & cooldown
	AMainPlayerController* Controller = Cast<AMainPlayerController>(CharReference->GetController());
	if (nullptr == WUIWidgetReference || nullptr == Controller) return;
	UIWidgetReference = CreateWidget<UUserWidget>(Controller, WUIWidgetReference);
	UIWidgetReference->AddToViewport();
	UIWidgetReference->SetVisibility(ESlateVisibility::Visible);

	UpdateCrossHairWithSpread(TraceSpreadMax);
	UpdateAmmoUI(CharReference->Arrows);

	CharReference->AimAssistPlayRate = AimAssistPlayRate;

	// If Static mesh obstacles in front of the player, adding the UI to the screen.
	if (nullptr == WCantHitWidget) return;
	CantHitWidget = CreateWidget<UUserWidget>(Controller, WCantHitWidget);
	CantHitWidget->AddToViewport();

	// Bow setting when idle
	HideArrow();
	DontHoldCable();
	CharReference->MaxArrows = MaxArrows;
	if (CharReference->Arrows > MaxArrows) CharReference->Arrows = MaxArrows;

	// Set Projectile Status
	SetArrowStatus(EArrowStatus::EAS_Normal);
}

void ABow::UpdateCrossHairWithSpread_Implementation(float CurrentSpread)
{
	CircleScale = FMath::Clamp(CurrentSpread / 2.0f, 0.2f, 1.8f);
}

void ABow::UpdateAmmoUI_Implementation(int32 Arrows)
{
	FString Str;
	
	if (Arrows < MaxArrows) Str = FString::FromInt(Arrows);
	else Str = FString::FromInt(MaxArrows);

	Str += "/";
	Str += FString::FromInt(MaxArrows);

	AmmoText = FText::FromString(Str);
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *AmmoText.ToString());
}

void ABow::CheckIfCanFire()
{
	if (IsAiming)
	{
		FVector ArrowSpawnLoc = ArrowSpawnLocation->GetComponentLocation();
		FVector CamForward = CharReference->FollowCamera->GetForwardVector();

		FVector StartLoc = ArrowSpawnLoc + CamForward * ArrowStartLocationOffset;
		FVector EndLoc = CameraTrace();

		TArray<AActor*> IgnoreActors;
		FHitResult HitResult;

		bool Result = UKismetSystemLibrary::SphereTraceSingle(
			GetWorld(),
			StartLoc,
			EndLoc,
			TraceRadius,
			ETraceTypeQuery::TraceTypeQuery1,
			false,
			IgnoreActors,
			EDrawDebugTrace::None,	// 디버그
			HitResult,
			true
			// 여기 밑에 3개는 기본 값으로 제공됨. 바꾸려면 적으면 됨.
			//, FLinearColor::Red
			//, FLinearColor::Green
			//, 5.0f
		);

		if (Result)
		{
			if (HitResult.GetActor() != HitActorObstacleCheck)	// 카메라의 끝과 화살이나가는 곳의 끝이 다를 때 canthit ui
			{
				AStaticMeshActor* Mesh = Cast<AStaticMeshActor>(HitResult.GetActor());
				if (Mesh)
				{
					CantHitWidgetLocation = HitResult.ImpactPoint;
					ShowCantHitWidget = true;
					return;
				}
			}
		}
	}

	// 위 모든 경우가 아닐 경우 dont show
	ShowCantHitWidget = false;
}

FVector ABow::CameraTrace()
{
	FVector CamLoc = CharReference->FollowCamera->GetComponentLocation();
	FVector CamForward = CharReference->FollowCamera->GetForwardVector();
	
	// 원뿔 모양으로 랜덤한 벡터를 구함 , 화살이 튀는 방향
	FVector RandVec = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(CamForward, TraceSpreadMin);
	
	FVector StartLoc = CamLoc + CamForward * ArrowStartLocationOffset;
	FVector EndLoc = CamLoc + RandVec * (ShootingDistanceMax / ObstacleMaxDistanceDivider);

	TArray<AActor*> IgnoreActors;
	FHitResult HitResult;

	bool Result = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		StartLoc,
		EndLoc,
		TraceRadius,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,	// 디버그
		HitResult,
		true
		// 여기 밑에 3개는 기본 값으로 제공됨. 바꾸려면 적으면 됨.
		//, FLinearColor::Red
		//, FLinearColor::Green
		//, 5.0f
	);

	HitActorObstacleCheck = HitResult.GetActor();

	if (Result)
	{
		return HitResult.ImpactPoint;
	}
	else 
	{
		return HitResult.TraceEnd;
	}
}

void ABow::HideArrow()
{
	ArrowMesh->SetHiddenInGame(true);
}

void ABow::ShowArrow()
{
	ArrowMesh->SetHiddenInGame(false);
}

void ABow::HoldCable()
{
	if (IsAiming)
	{
		TopCable->SetAttachEndTo(CharReference, "Mesh", "middle_01_r");
		BottomCable->SetAttachEndTo(CharReference, "Mesh", "middle_01_r");
	}
}

void ABow::DontHoldCable()
{
	TopCable->SetAttachEndTo(this, "bow", "BowIdle");
	BottomCable->SetAttachEndTo(this, "bow", "BowIdle");
}

void ABow::EquippedChanged(bool NewHidden)
{
	if (NewHidden) HideArrow();
	else ShowArrow();

	ArrowInvisibility = NewHidden;
}

void ABow::UpdateArrowLocationRotation()
{
	if (ArrowInvisibility) return;

	FRotator LookAtRotation;

	// Constantly Updating arrow rotation
	if (IsAiming)
	{
		LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ArrowScene->GetComponentLocation(), Bow->GetComponentLocation());
		FRotator NewRotation = UKismetMathLibrary::RInterpTo(ArrowScene->GetComponentRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), 30.f);
		ArrowScene->SetWorldRotation(NewRotation);
	}

	// Update Arrow Attach to Location
	if (IsAiming)
	{
		if (DoOnce) 
		{
			DoOnce = false;
			ArrowScene->AttachToComponent(CharReference->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false), "hand_R");
			ArrowScene->SetWorldRotation(LookAtRotation);
		}
	}
	else
	{
		if (!DoOnce)
		{
			DoOnce = true;
			ArrowScene->AttachToComponent(Bow, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false), "BowIdle");
			ArrowScene->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(ArrowScene->GetComponentLocation(), Bow->GetComponentLocation()));
		}
	}
}

void ABow::AimStart()
{
	IsAiming = true;

	AimChange(true);
}

void ABow::AimStop()
{
	IsAiming = false;

	if (IsDrawing) DrawStop();
	AimChange(false);
}

void ABow::AimChange(bool Aiming)
{
	if (Aiming)
	{
		//HideOrShow(false);
		FadeInPostProcess(1.f);
		FadeInCrosshair();
		CameraArmLengthChanges(AimSubstractionArmLength);

		TopCable->SetAttachEndTo(CharReference, "Mesh", "middle_01_r");
		BottomCable->SetAttachEndTo(CharReference, "Mesh", "middle_01_r");

		//if (CharReference->GetCharacterMovement()->IsFalling())	SlowMotion();

		if (TryingToDraw) DrawStart();

		//Change rotation rules for the character.
		CharReference->GetCharacterMovement()->bOrientRotationToMovement = false;
		CharReference->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		CharReference->bUseControllerRotationYaw = true;

		CharReference->GetCharacterMovement()->MaxWalkSpeed = CharReference->WalkingSpeed;
	}
	else 
	{
		HideOrShow(true);
		FadeOutPostProcess(1.f);
		FadeOutCrosshair();
		CameraArmLengthChangesRevert();

		TopCable->SetAttachEndTo(this, "bow", "BowIdle");
		BottomCable->SetAttachEndTo(this, "bow", "BowIdle");

		//StopSlowMotion();

		StopDraw();

		//Change rotation rules for the character.
		CharReference->GetCharacterMovement()->bOrientRotationToMovement = true;
		CharReference->GetCharacterMovement()->bUseControllerDesiredRotation = false;
		CharReference->bUseControllerRotationYaw = false;

		if(CharReference->GetMovementStatus() != EMovementStatus::EMS_Sprinting)
			CharReference->GetCharacterMovement()->MaxWalkSpeed = CharReference->RunningSpeed;
		
		InteruptedDrawShoot();
	}
}

void ABow::HideOrShow_Implementation(bool Hide){}
void ABow::FadeInPostProcess_Implementation(float PlayRatePostProcess){}
void ABow::FadeOutPostProcess_Implementation(float PlayRatePostProcess){}
void ABow::CameraArmLengthChanges_Implementation(float SubstractionArmLength){}
void ABow::CameraArmLengthChangesRevert_Implementation() {}
void ABow::FadeInCrosshair_Implementation() {}
void ABow::FadeOutCrosshair_Implementation() {}
void ABow::SlowMotion_Implementation() {}
void ABow::StopSlowMotion_Implementation() {}
void ABow::ChangeFOV_Implementation(float FOVToSubtract, float FOVChangesPlayRate) {}
void ABow::RestoreFOV_Implementation(float FOVChangesPlayRate) {}
void ABow::StartDamageNumbersTimeline_Implementation(float DamageSettingsPlayRate) {}
void ABow::StopDamageNumbersTimeline_Implementation() {}

void ABow::DrawStart()
{
	TryingToDraw = true;
	IsDrawing = true;

	DrawingChanges();
}

void ABow::DrawStop()
{
	TryingToDraw = false;
	IsDrawing = false;

	DrawingChanges();
}

void ABow::DrawingChanges()
{
	if (IsDrawing)
	{
		ChangeFOV(SubtractionFOV, DrawChangesPlayRate);

		if (CharReference->GetCharacterMovement()->IsFalling())	SlowMotion();

		FadeInPostProcess(DrawChangesPlayRate);

		StartDamageNumbersTimeline(DrawChangesPlayRate);
	}
	else 
	{
		StopDraw();
	}
}

void ABow::StopDraw()
{
	RestoreFOV(ShootCameraChangesPlayRate);

	StopSlowMotion();

	FadeOutPostProcess(1.f);

	UpdateCrossHairWithSpread(TraceSpreadMax);

	StopDamageNumbersTimeline();

	// Reset Draw
	CharReference->Draw = 0.f;
}

void ABow::InteruptedDrawShoot()
{
	IsAiming = false;

	Shoot();
}

void ABow::DrawShoot()
{
	Shoot();
	DrawStop();

	if (RainOfArrowsActive)
	{
		StartRainOfArrowsCooldown();
		SetRainOfArrowsActive(false);
	}
	else if (IceArrowActive)
	{
		StartIceArrowCooldown();
		SetIceArrowActive(false);
	}
	else if (DetectionArrowActive)
	{
		StartDetectionArrowCooldown();
		SetDetectionArrowActive(false);
	}
}

void ABow::Shoot()
{
	CharReference->Draw = 0.f;

	if (!IsAiming) return;
	if (!CanShootAndSubtractAmmo()) return;

	CharReference->PShoot->Activate(true);

	FTransform Transform = CalculateProjectileInfo();

	SpawnProjectile(Transform);
}

bool ABow::CanShootAndSubtractAmmo()
{
	if (IsAiming)
	{
		if (CharReference->Arrows > 0)
		{
			CharReference->SubtractArrows(1);
			return true;
		}
	}

	return false;
}

FTransform ABow::CalculateProjectileInfo()
{
	FVector CamLoc = CharReference->FollowCamera->GetComponentLocation();
	FVector CamForward = CharReference->FollowCamera->GetForwardVector();
	FVector RandVec = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(CamForward, TraceSpread);
	float CurrentProjectileSpeed = FMath::Clamp(ProjectileSpeed, 0.f, 6000.f);
	
	FVector StartLoc = CamLoc + CamForward * ArrowStartLocationOffset;
	FVector EndLoc = CamLoc + RandVec * CurrentProjectileSpeed;

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Emplace(CharReference);

	FHitResult HitResult;

	bool Result = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		StartLoc,
		EndLoc,
		TraceRadius,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,	// 디버그
		HitResult,
		true
		// 여기 밑에 3개는 기본 값으로 제공됨. 바꾸려면 적으면 됨.
		//, FLinearColor::Red
		//, FLinearColor::Green
		//, 5.0f
	);

	if (Result)
	{
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ArrowSpawnLocation->GetComponentLocation(), HitResult.ImpactPoint);
		FTransform Transform;
		Transform.SetLocation(ArrowSpawnLocation->GetComponentLocation());
		Transform.SetRotation(FQuat(LookAtRotation));
		
		return Transform;
	}
	else 
	{
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ArrowSpawnLocation->GetComponentLocation(), EndLoc);
		FTransform Transform;
		Transform.SetLocation(ArrowSpawnLocation->GetComponentLocation());
		Transform.SetRotation(FQuat(LookAtRotation));

		return Transform;
	}
}

void ABow::SpawnProjectile(FTransform SpawnTransform)
{
	FireEffects();
	TSubclassOf<AArrow> ArrowType;
	switch (GetArrowStatus())
	{
	case EArrowStatus::EAS_Normal:
		ArrowType = Normal;
		break;
	case EArrowStatus::EAS_Rain:
		ArrowType = Rain;
		break;
	case EArrowStatus::EAS_Ice:
		ArrowType = Ice;
		break;
	case EArrowStatus::EAS_Detection:
		ArrowType = Detection;
		break;
	}

	if (nullptr == ArrowType) return;
	
	AArrow* Arrow = GetWorld()->SpawnActorDeferred<AArrow>(ArrowType, SpawnTransform);
		
	Arrow->BowReference = this;
	Arrow->Damage = Damage;
	Arrow->ProjectileSpeed = ProjectileSpeed;

	Arrow->FinishSpawning(SpawnTransform);
}

void ABow::FireEffects()
{
	if (ShootSound) UGameplayStatics::PlaySoundAtLocation(this, ShootSound, Bow->GetComponentLocation());
	
	if (FireParticle) UGameplayStatics::SpawnEmitterAtLocation(this, FireParticle, DrawSound->GetComponentLocation());

	StopDamageNumbersTimeline();
}

void ABow::DamageEvent(FHitResult HitResult, float BaseDamage, TSubclassOf<UDamageType> ArrowDamageType)
{
	
	// Arrow SkillType Check
	if (ArrowDamageType == RainType)
		RainOfArrows(HitResult);
	else if (ArrowDamageType == IceType)
		IceArrow(HitResult);
	else if (ArrowDamageType == DetectionType)
		DetectionArrow(HitResult.ImpactPoint);


	if (!HitResult.bBlockingHit) return;

	UGameplayStatics::ApplyPointDamage(
		HitResult.GetActor(),
		BaseDamage,
		Bow->GetComponentLocation(),
		HitResult,
		CharReference->GetController(),
		CharReference,
		ArrowDamageType
		);

	PlayParticleAtLocation(HitResult);
	PlaySoundAtLocation(HitResult);

	// 충격량 전달
	if (HitResult.GetComponent())
		HitResult.GetComponent()->AddRadialImpulse(HitResult.ImpactPoint, 500.f, 50000.f, ERadialImpulseFalloff::RIF_Constant);
	

	if (HitResult.GetComponent()->IsAnySimulatingPhysics()) return;

	SpawnArrowPickUp(HitResult);

	// Arrow가 Bone에 맞았다면 Bone에 붙임
	if (HitResult.BoneName != "None")
	{
		if (ArrowPickUp && HitResult.GetComponent())
		{
			ArrowPickUp->AttachToComponent(HitResult.GetComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true), HitResult.BoneName);

			// 맞았을 때 상처 낼 것인지
			if (ArrowDamageType != IceType)
			{
				AEnemy* Enemy = Cast<AEnemy>(HitResult.GetActor());
				if (Enemy)
				{
					for(int i=0; i<2; i++)	// 너무 연해서 두번
						UGameplayStatics::SpawnDecalAttached(
							BurningDecal,
							FVector(20.f),
							Enemy->GetMesh(),
							HitResult.BoneName,
							Enemy->GetMesh()->GetSocketLocation(HitResult.BoneName),
							FRotator(0.f, -90.f, 0.f),
							EAttachLocation::KeepWorldPosition,
							5.f
							);
				}
			}
		}
	}

	ArrowPickUp = nullptr;
}

void ABow::PlayParticleAtLocation(FHitResult Hit)
{
	UParticleSystem* ImpactEffect = DefaultImpactEffect;
	FVector SpawnEffectLoc = Hit.TraceEnd;
	if (Hit.bBlockingHit)
	{
		switch (Hit.PhysMaterial.Get()->SurfaceType)
		{
		case EPhysicalSurface::SurfaceType1:
			ImpactEffect = BrickImpactEffect;
			break;
		case EPhysicalSurface::SurfaceType2:
			ImpactEffect = DirtImpactEffect;
			break;
		case EPhysicalSurface::SurfaceType3:
			ImpactEffect = WoodImpactEffect;
			break;
		case EPhysicalSurface::SurfaceType4:
			ImpactEffect = BloodImpactEffect;
			break;
		case EPhysicalSurface::SurfaceType5:
			ImpactEffect = WaterImpactEffect;
			break;
		case EPhysicalSurface::SurfaceType6:
			ImpactEffect = GrassImpactEffect;
			break;
		}
		SpawnEffectLoc = Hit.ImpactPoint;
	}

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, SpawnEffectLoc, FRotator(0.f), true);
}

void ABow::PlaySoundAtLocation(FHitResult Hit)
{
	USoundCue* ImpactSound = DefaultImpactSound;
	switch (Hit.PhysMaterial.Get()->SurfaceType)
	{
	case EPhysicalSurface::SurfaceType1:
		ImpactSound = BrickImpactSound;
		break;
	case EPhysicalSurface::SurfaceType2:
		ImpactSound = DirtImpactSound;
		break;
	case EPhysicalSurface::SurfaceType3:
		ImpactSound = WoodImpactSound;
		break;
	case EPhysicalSurface::SurfaceType4:
		ImpactSound = BloodImpactSound;
		break;
	case EPhysicalSurface::SurfaceType5:
		ImpactSound = WaterImpactSound;
		break;
	case EPhysicalSurface::SurfaceType6:
		ImpactSound = GrassImpactSound;
		break;
	}

	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.ImpactPoint);
}

void ABow::SpawnArrowPickUp(FHitResult HitResult)
{
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(HitResult.ImpactPoint);

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CharReference->GetMesh()->GetComponentLocation(), HitResult.Location + HitResult.Normal);
	float RandValue = UKismetMathLibrary::RandomFloatInRange(-10.f, 20.f);

	FRotator TransformRotation;
	TransformRotation.Roll = LookAtRotation.Roll + 90.f;
	TransformRotation.Pitch = LookAtRotation.Pitch - 40.f + RandValue;
	TransformRotation.Yaw = LookAtRotation.Yaw;

	SpawnTransform.SetRotation(FQuat(TransformRotation));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ArrowPickUp = GetWorld()->SpawnActor<AArrowPickUp>(ArrowPickUpAsset, SpawnTransform, SpawnParams);
}

void ABow::SetRainOfArrowsActive(bool Condition)
{
	if (Condition)
		StartRainOfArrows();
	else
		StopRainOfArrows();
}

void ABow::StartRainOfArrows()
{
	RainOfArrowsActive = true;
	bArrowSkillActive = true;
	SetArrowStatus(EArrowStatus::EAS_Rain);
	RainOfArrowsEffectActivate();
}

void ABow::StopRainOfArrows()
{
	RainOfArrowsActive = false;
	bArrowSkillActive = false;
	SetArrowStatus(EArrowStatus::EAS_Normal);
	RainOfArrowsEffectDeactivate();
}

void ABow::StartRainOfArrowsCooldown()
{
	RainOfArrowsCoolDown = true;

	RainOfArrowsCoolDownUI = 0.f;

	FTimerHandle RainOfArrowsCoolDownHandle;
	GetWorld()->GetTimerManager().SetTimer(RainOfArrowsCoolDownHandle, FTimerDelegate::CreateLambda([&]()
		{
			RainOfArrowsCoolDown = false;
		}), RainOfArrowsCoolDownTime, false);
}

void ABow::RainOfArrows(FHitResult Hit)
{
	//가장 가까운 지면을 찾음
	FVector StartLoc;
	if (Hit.bBlockingHit) StartLoc = Hit.ImpactPoint;
	else StartLoc = Hit.TraceEnd;
	StartLoc += FVector(10.f, 0.f, 0.f);

	FVector EndLoc = StartLoc - FVector(0.f, 0.f, 5000.f);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	TEnumAsByte<EObjectTypeQuery> Target = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic);
	ObjectTypes.Add(Target);

	TArray<AActor*> IgnoreActors;
	FHitResult HitResult;

	bool Result = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		StartLoc,
		EndLoc,
		ObjectTypes,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,	// 디버그
		HitResult,
		true
		// 여기 밑에 3개는 기본 값으로 제공됨. 바꾸려면 적으면 됨.
		//, FLinearColor::Red
		//, FLinearColor::Green
		//, 5.0f
	);
	
	FVector DamageEndLoc;
	if (HitResult.bBlockingHit) DamageEndLoc = HitResult.Location;
	else DamageEndLoc = HitResult.ImpactPoint;
	DamageEndLoc.Z -= 2500.f;

	if (RainOfArrowsBurst) UGameplayStatics::SpawnEmitterAtLocation(this, RainOfArrowsBurst, Hit.ImpactPoint);
	if (RainOfArrowsParticle) UGameplayStatics::SpawnEmitterAtLocation(this, RainOfArrowsParticle, Hit.ImpactPoint);

	RainOfArrowsHit(StartLoc, DamageEndLoc);
}

void ABow::RainOfArrowsEffectActivate_Implementation() {}
void ABow::RainOfArrowsEffectDeactivate_Implementation() {}
void ABow::SetUIRainOfArrowsCoolDown_Implementation() {}
void ABow::RainOfArrowsHit_Implementation(FVector StartLoc, FVector Location) {}

void ABow::SetIceArrowActive(bool Condition)
{
	if (Condition)
		StartIceArrow();
	else
		StopIceArrow();
}

void ABow::StartIceArrow()
{
	IceArrowActive = true;
	bArrowSkillActive = true;
	SetArrowStatus(EArrowStatus::EAS_Ice);
	IceArrowEffectActivate();
}

void ABow::StopIceArrow()
{
	IceArrowActive = false;
	bArrowSkillActive = false;
	SetArrowStatus(EArrowStatus::EAS_Normal);
	IceArrowEffectDeactivate();
}

void ABow::StartIceArrowCooldown()
{
	IceArrowCoolDown = true;

	IceArrowCoolDownUI = 0.f;

	FTimerHandle IceArrowCoolDownHandle;
	GetWorld()->GetTimerManager().SetTimer(IceArrowCoolDownHandle, FTimerDelegate::CreateLambda([&]()
		{
			IceArrowCoolDown = false;
		}), IceArrowCoolDownTime, false);
}

void ABow::IceArrow(FHitResult Hit)
{
	IceTarget = Cast<AEnemy>(Hit.GetActor());
	if (IceTarget)
	{
		if (IceArrowParticle) UGameplayStatics::SpawnEmitterAtLocation(this, IceArrowParticle, IceTarget->GetActorLocation());
		if (IceArrowSound) UGameplayStatics::PlaySoundAtLocation(this, IceArrowSound, IceTarget->GetActorLocation());

		// Start Freeze
		FTimerHandle StartFreezeHandle;
		GetWorld()->GetTimerManager().SetTimer(StartFreezeHandle, FTimerDelegate::CreateLambda([&]()
			{
				OriginalMaterials = IceTarget->GetMesh()->GetMaterials();
				if (IceMaterial)
				{
					int32 i = 0;
					for (auto OriginalMaterial : OriginalMaterials)
					{
						IceTarget->GetMesh()->SetMaterial(i, IceMaterial);
						i++;
					}
				}
				IceTarget->GetCharacterMovement()->DisableMovement();
				IceTarget->GetMesh()->bPauseAnims = true;
				IceTarget->StopAnim();

				// End Freeze
				FTimerHandle EndFreezeHandle;
				GetWorld()->GetTimerManager().SetTimer(EndFreezeHandle, FTimerDelegate::CreateLambda([&]()
					{
						int32 i = 0;
						for (auto OriginalMaterial : OriginalMaterials)
						{
							IceTarget->GetMesh()->SetMaterial(i, OriginalMaterial);
							i++;
						}
						IceTarget->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
						IceTarget->GetMesh()->bPauseAnims = false;
						if (IceTarget->bFreezing) IceTarget->bFreezing = false;

						IceTarget = nullptr;
						OriginalMaterials.Empty();
					}), 3.f, false);

			}), 0.2f, false);
	}
}

void ABow::IceArrowEffectActivate_Implementation() {}
void ABow::IceArrowEffectDeactivate_Implementation() {}
void ABow::SetUIIceArrowCoolDown_Implementation() {}


void ABow::SetDetectionArrowActive(bool Condition)
{
	if (Condition)
		StartDetectionArrow();
	else
		StopDetectionArrow();
}

void ABow::StartDetectionArrow()
{
	DetectionArrowActive = true;
	bArrowSkillActive = true;
	SetArrowStatus(EArrowStatus::EAS_Detection);
	DetectionArrowEffectActivate();
}

void ABow::StopDetectionArrow()
{
	DetectionArrowActive = false;
	bArrowSkillActive = false;
	SetArrowStatus(EArrowStatus::EAS_Normal);
	DetectionArrowEffectDeactivate();
}

void ABow::StartDetectionArrowCooldown()
{
	DetectionArrowCoolDown = true;

	DetectionArrowCoolDownUI = 0.f;

	FTimerHandle DetectionArrowCoolDownHandle;
	GetWorld()->GetTimerManager().SetTimer(DetectionArrowCoolDownHandle, FTimerDelegate::CreateLambda([&]()
		{
			DetectionArrowCoolDown = false;
		}), DetectionArrowCoolDownTime, false);
}

void ABow::DetectionArrow(FVector HitLoc)
{
	if (DetectionArrowParticle) UGameplayStatics::SpawnEmitterAtLocation(this, DetectionArrowParticle, HitLoc, FRotator::ZeroRotator, FVector(2.f));

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	TEnumAsByte<EObjectTypeQuery> Target = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2);
	ObjectTypes.Add(Target);	//Enemy

	TArray<AActor*> IgnoreActors;
	
	bool Result = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		HitLoc,
		HitLoc,
		10000.f,
		ObjectTypes,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,	// 디버그
		DetectionHitResults,
		true
		// 여기 밑에 3개는 기본 값으로 제공됨. 바꾸려면 적으면 됨.
		//, FLinearColor::Red
		//, FLinearColor::Green
		//, 5.0f
	);

	if (!Result) return;

	for (FHitResult HitResult : DetectionHitResults)
	{
		AEnemy* Enemy = Cast<AEnemy>(HitResult.GetActor());
		if (Enemy) Enemy->GetMesh()->SetRenderCustomDepth(true);
	}

	FTimerHandle DetectionHandle;
	GetWorld()->GetTimerManager().SetTimer(DetectionHandle, FTimerDelegate::CreateLambda([&]()
		{
			for (FHitResult HitResult : DetectionHitResults)
			{
				AEnemy* Enemy = Cast<AEnemy>(HitResult.GetActor());
				if (Enemy) Enemy->GetMesh()->SetRenderCustomDepth(false);
			}
			DetectionHitResults.Empty();

		}), DetectionTime, false);
}

void ABow::DetectionArrowEffectActivate_Implementation() {}
void ABow::DetectionArrowEffectDeactivate_Implementation() {}
void ABow::SetUIDetectionArrowCoolDown_Implementation() {}
