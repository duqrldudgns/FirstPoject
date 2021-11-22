// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/DecalComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Math/TransformNonVectorized.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "MainPlayerController.h"
#include "MainAnimInstance.h"
#include "Sound/SoundCue.h"
#include "FirstSaveGame.h"
#include "ShootingSkill.h"
#include "DamageNumbers.h"
#include "PositionDecal.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ItemStorage.h"
#include "Cpt_FootIK.h"
#include "Weapon.h"
#include "Enemy.h"
#include "Bow.h"

// Sets default values
AMain::AMain()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FootIK = CreateDefaultSubobject<UCpt_FootIK>(TEXT("FootIK"));

	// Create SwordAttached
	SwordAttached = CreateDefaultSubobject<UChildActorComponent>(TEXT("SwordAttached"));
	SwordAttached->SetupAttachment(GetMesh(), "Sword_Back");		// Attach the camera to the end of the boom and let the boom adjust to match 

	BowAttached = CreateDefaultSubobject<UChildActorComponent>(TEXT("BowAttached"));
	BowAttached->SetupAttachment(GetMesh(), "Bow_Back");

	Quiver = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Quiver"));
	Quiver->SetupAttachment(GetMesh(), "QuiverSocket");
	
	QuiverArrows = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("QuiverArrows"));
	QuiverArrows->SetupAttachment(Quiver);
	QuiverArrows->SetWorldRotation(FRotator(30.f, 0.f, 0.f));
	QuiverArrows->InstancingRandomSeed = 21757;
	QuiverArrows->SetEnableGravity(false);
	QuiverArrows->SetGenerateOverlapEvents(false);
	QuiverArrows->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	QuiverArrows->SetCollisionProfileName("NoCollision");
	AddInstanceQuiverArrows();

	// Create Character Select Decal
	SelectDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("SelectDecal"));
	SelectDecal->SetupAttachment(GetRootComponent());
	SelectDecal->SetVisibility(false);
	SelectDecal->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	static ConstructorHelpers::FObjectFinder<UMaterial> Decal(TEXT("Material'/Game/GameplayMechanics/test/CharaSelectDecal/CharSelectDecal_MT.CharSelectDecal_MT'"));
	if (Decal.Succeeded())
	{
		SelectDecal->SetDecalMaterial(Decal.Object);
	}
	
	// Create Camera Boom (pulls towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;	//Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true;		// Rotate arm based on controller 컨트롤러 기반으로 회전
	CameraBoom->bEnableCameraLag = true;	// 부드럽게 전환

	// Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(34.f, 88.f);

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);		// Attach the camera to the end of the boom and let the boom adjust to match 
	FollowCamera->bUsePawnControlRotation = false;		// 이미 springarm이 true이기 때문에 컨트롤러에 의존하지않음

	FollowCameraZoomIn = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCameraZoomIn"));
	FollowCameraZoomIn->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCameraZoomIn->SetRelativeLocation(FVector(300.f, 80.f, 55.f));
	FollowCameraZoomIn->bUsePawnControlRotation = false;
	
	PShoot = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PShoot"));
	PShoot->SetupAttachment(FollowCamera);
	PShoot->SetRelativeLocation(FVector(100.f, 0.f, 0.f));
	PShoot->SetAutoActivate(false);

	ArrowsVisualize = CreateDefaultSubobject<USceneComponent>(TEXT("ArrowsVisualize"));
	ArrowsVisualize->SetupAttachment(GetRootComponent());

	// Set our turn rates for input , 1초동안 키를 누르고 있으면 회전하는 양
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// 참이면 이 Pawn의 yaw는 PlayerController에 의해 제어되는 경우 컨트롤러의 ControlRotation yaw와 일치하도록 업데이트
	// Don't rotate when the controller rotates
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;	//Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f);	// ...at this rotation rate , yaw에 해당하는 부분의 값을 올리면 회전속도가 빨라짐
	GetCharacterMovement()->JumpZVelocity = 650.f;	// 점프 폭
	GetCharacterMovement()->AirControl = 0.2f;	// 점프 시 제어강도

	JumpMaxHoldTime = 0.1f;
	JumpMaxCount = 2;

	MaxHealth = 250.f;
	Health = 250.f;
	MaxStamina = 100.f;
	Stamina = 50.f;
	Coins = 0;

	WalkingSpeed = 300.f;
	RunningSpeed = 600.f;
	SprintingSpeed = 1000.f;

	bShiftKeyDown = false;
	bSwordEquipKeyDown = false;
	bLMBDown = false;
	bRMBDown = false;
	bAttacking = false;
	bInterpToEnemy = false;
	bHasCombatTarget = false;
	bMovingForward = false;
	bMovingRight = false;
	bEscDown = false;
	bComboAttackInput = false;
	bGuardAccept = false;
	bFirstSkillKeyDown = false;
	bSecondSkillKeyDown = false;
	bThirdSkillKeyDown = false;
	bFourthSkillKeyDown = false;
	bSkillCasting = false;
	bArmedBridgeIng = false;
	bTestKeyDown = false;
	bDodgeIng = false;
	bGotHit = false;

	//Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;
	ArmedStatus = EArmedStatus::EAS_Normal;

	StaminaDrainRate = 20.f;
	MinSprintStamina = 30.f;

	InterpSpeed = 15.f;

	ComboCnt = 0;

	WaveSkillDamage = 25.f;

	FootRotationL = FRotator::ZeroRotator;
	FootRotationR = FRotator::ZeroRotator;
	HipOffset = 0.f;
	FootOffsetL = 0.f;
	FootOffsetR = 0.f;

	FollowCameraInitLoc = FollowCamera->GetRelativeLocation();

	// Bow
	DefaultFieldOfView = FollowCamera->FieldOfView;
	DefaultArmLength = CameraBoom->TargetArmLength;
	TraceRadius = 50.f;
	MaxAimAssistDistance = 2000.f;
	MaxTraceRadius = 100.f;
	AimAssistPlayRate = 1.f;
	Arrows = 99999;	
	CanDraw = true;	
	MaxArrows = 10;	

	FirstSkillCoolDownUI = 1.f;
	SecondSkillCoolDownUI = 1.f;
	ThirdSkillCoolDownUI = 1.f;
	FourthSkillCoolDownUI = 1.f;

	FirstSkillCoolDownTime = 3.f;
	SecondSkillCoolDownTime = 5.f;
	ThirdSkillCoolDownTime = 15.f;
	FourthSkillCoolDownTime = 10.f;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	// 충돌 유형 지정
	GetCapsuleComponent()->SetCollisionProfileName("Player");

	//컨트롤러를 세팅해야 누가 피해를 주는지 알 수 있고 피해를 줄 수 있음
	SetInstigator(GetController());

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	FString Map = GetWorld()->GetMapName();
	Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	// 장비를 장착할 때 그 캐릭터의 컨트롤러를 세팅해야 누가 피해를 주는지 알 수 있고 피해를 줄 수 있음

	if (Map != "SunTemple")
	{
		LoadGameNoSwitch();
	}

	BowInitSet();
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Alive()) return;

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown && (bMovingForward || bMovingRight))
		{
			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
			}
			Stamina -= DeltaStamina;
			SetMovementStatus(EMovementStatus::EMS_Sprinting);
		}
		else {
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown && (bMovingForward || bMovingRight))
		{
			if (Stamina - DeltaStamina <= 0.f)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
			}
			else
			{
				Stamina -= DeltaStamina;
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
		}
		else
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
			}
			Stamina += DeltaStamina;
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown)
		{
			Stamina = 0.f;
		}
		else
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	case EStaminaStatus::ESS_ExhaustedRecovering:
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
		}
		Stamina += DeltaStamina;
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	default:
		;
	}

	// 공격을 시작할 때 주변에 적이 있다면 방향전환
	if (bInterpToEnemy && CombatTarget)		//Interp Rotation
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);	// 스무스하게 회전

		SetActorRotation(InterpRotation);
	}

	// HP바 보이기 위해 위치 얻어옴
	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}

	// 무기 갖고 있다면 Guard 상태로 전환
	if (bRMBDown)
	{
		if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

		if (EquippedWeapon && CanAction() && !GetMovementComponent()->IsFalling())
		{
			SetMovementStatus(EMovementStatus::EMS_Guard);
		}
	}

	//if (Aiming) 
	FRotator CurrentRotation(AimPitch, AimYaw, 0.f);
	FRotator TargetRotation = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());

	FRotator InterpRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);
	AimPitch = InterpRotation.Pitch;
	AimYaw = InterpRotation.Yaw;

	CameraChangeInterp();

	LockOnTargetCameraInterp();

	// Blackhole Skill
	ShowBlackholeRange();

	// CloneAttack Skill
	CloneAttackCast();

	FirstSkillCoolDownReturn();
	SecondSkillCoolDownReturn();
	ThirdSkillCoolDownReturn();
	FourthSkillCoolDownReturn();
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent); // 유효한지 확인 후 false 반환하면 실행 중지

	//해당하는 키를 누르면 이에 맞는 함수 호출
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMain::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("SwordEquip", IE_Pressed, this, &AMain::SwordEquipKeyDown);
	PlayerInputComponent->BindAction("SwordEquip", IE_Released, this, &AMain::SwordEquipKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("RMB", IE_Pressed, this, &AMain::RMBDown);
	PlayerInputComponent->BindAction("RMB", IE_Released, this, &AMain::RMBUp);

	PlayerInputComponent->BindAction("Esc", IE_Pressed, this, &AMain::EscDown);
	PlayerInputComponent->BindAction("Esc", IE_Released, this, &AMain::EscUp);

	PlayerInputComponent->BindAction("FirstSkill", IE_Pressed, this, &AMain::FirstSkillKeyDown);
	PlayerInputComponent->BindAction("FirstSkill", IE_Released, this, &AMain::FirstSkillKeyUp);

	PlayerInputComponent->BindAction("SecondSkill", IE_Pressed, this, &AMain::SecondSkillKeyDown);
	PlayerInputComponent->BindAction("SecondSkill", IE_Released, this, &AMain::SecondSkillKeyUp);

	PlayerInputComponent->BindAction("ThirdSkill", IE_Pressed, this, &AMain::ThirdSkillKeyDown);
	PlayerInputComponent->BindAction("ThirdSkill", IE_Released, this, &AMain::ThirdSkillKeyUp);

	PlayerInputComponent->BindAction("FourthSkill", IE_Pressed, this, &AMain::FourthSkillKeyDown);
	PlayerInputComponent->BindAction("FourthSkill", IE_Released, this, &AMain::FourthSkillKeyUp);

	PlayerInputComponent->BindAction("Combo1", IE_Pressed, this, &AMain::Combo1KeyDown);
	PlayerInputComponent->BindAction("Combo1", IE_Released, this, &AMain::Combo1KeyUp);

	PlayerInputComponent->BindAction("Combo2", IE_Pressed, this, &AMain::Combo2KeyDown);
	PlayerInputComponent->BindAction("Combo2", IE_Released, this, &AMain::Combo2KeyUp);

	PlayerInputComponent->BindAction("BowEquip", IE_Pressed, this, &AMain::BowEquipKeyDown);
	PlayerInputComponent->BindAction("BowEquip", IE_Released, this, &AMain::BowEquipKeyUp);

	PlayerInputComponent->BindAction("Test", IE_Pressed, this, &AMain::TestKeyDown);
	PlayerInputComponent->BindAction("Test", IE_Released, this, &AMain::TestKeyUp);

	PlayerInputComponent->BindAction("LockOnTarget", IE_Pressed, this, &AMain::LockOnTargetKeyDown);
	PlayerInputComponent->BindAction("LockOnTarget", IE_Released, this, &AMain::LockOnTargetKeyUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);

	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

}

bool AMain::CanMove(float Value)
{
	return Controller != nullptr && Value != 0.0f &&
		!MainPlayerController->bPauseMenuVisible
		&& Alive() && !bAttacking && !bSkillCasting && !bGotHit;
}

bool AMain::CanAction()
{
	return Alive() && !bAttacking && !bSkillCasting && !bGotHit &&
		!bArmedBridgeIng && !bDodgeIng && GetMovementStatus() != EMovementStatus::EMS_Guard;
}

void AMain::MoveForward(float Value)
{
	bMovingForward = false;		// 이렇게 해도 움직일 시 true로 인정 되는 듯,  매 틱 실행되니까
	if (CanMove(Value))	// 공격시 움직임 제어
	{
		// 닷지 키도 입력중이면 닷지 시전 , 아니면 움직임

		bMovingForward = true;

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();		// 변경 할 일이 없기때문에 const  ,  컨트롤러가 향하는 방향을 알려주는 rotation을 반환
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);		// 우리는 세계의 수평면에서 어떤 방향을 향하는지가 중요, 하나의 축을 가져온다 생각

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);	// Matrix(Yam) = 하나의 축을 기반으로 플레이어의 local방향을 가져옴, Get(X) 그 중에서 X방향을 가져옴 = 플레이어의 정면, 앞
		AddMovementInput(Direction, Value);

	}
}

void AMain::MoveRight(float Value)
{
	bMovingRight = false;
	if (CanMove(Value))	// 공격시 움직임 제어
	{
		bMovingRight = true;

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);	// X가 정면이라면 Y는 좌우 측면
		AddMovementInput(Direction, Value);

	}
}

void AMain::Turn(float Value)
{
	AddControllerYawInput(Value); // 플레이어컨트롤러의 Yaw값을 조정
}

void AMain::LookUp(float Value)
{
	AddControllerPitchInput(Value); // 플레이어컨트롤러의 Pitch값을 조정
}

void AMain::TurnAtRate(float Rate)
{
	//입력을 받아 축에서 컨트롤러를 구체적으로 회전시키는 기능
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());		// 틱함수 외부에서 델타시간을 얻는 방법
}

void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMain::Jump()
{
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;	// 메뉴창 실행중이라면 리턴

	if (CanAction())
	{
		ACharacter::Jump();	// =  Super::Jump();
	}
}

void AMain::StopJumping()
{
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;	// 메뉴창 실행중이라면 리턴

	if (CanAction())
	{
		ACharacter::StopJumping();	// =  Super::StopJumping();
	}
}

void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}

void AMain::IncrementHealth(float Amount)
{
	Health += Amount;
	if (Health > MaxHealth) Health = MaxHealth;
}
void AMain::DecrementHealth(float Amount)
{
	Health -= Amount;
	if (Health <= 0.f) Die();
}

void AMain::DecrementHealth(float Amount, AActor* DamageCauser)
{
	Health -= Amount;

	if (Health <= 0.f)
	{
		Die();
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy)
			{
				Enemy->bHasValidTarget = false;
			}
		}
	}
}

bool AMain::Alive()	// 살아있으면 true
{
	return GetMovementStatus() != EMovementStatus::EMS_Dead;
}

void AMain::Die()
{
	if (!Alive() && MovementStatus == EMovementStatus::EMS_Dead) return;

	SetMovementStatus(EMovementStatus::EMS_Dead);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//애니메이션 인스턴스를 가져옴
	if (AnimInstance && DamagedMontage)
	{
		AnimInstance->Montage_Play(DamagedMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"), DamagedMontage);
	}
}

void AMain::Resurrection()
{
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;			// 애니메이션 정지
	GetMesh()->bNoSkeletonUpdate = true;	// 스켈레톤 업데이트 정지
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	if (Aiming) return;

	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else if (MovementStatus == EMovementStatus::EMS_Normal)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
	else if (MovementStatus == EMovementStatus::EMS_Guard)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
	}
}

void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMain::LMBDown()
{
	bLMBDown = true;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;


	if (EquippedWeapon && CanAction())
	{
		if (bBlackholeKeyDown)
		{
			BlackholeSKillCast();
		}
		else if (GetCharacterMovement()->IsFalling()) {	// 공중 공격
			AirAttack();
		}
		else {	// 지상 공격
			Attack();
		}

		bComboAttackInput = false;	//@@@@@ FIX TODO : 주석처리 시 첫 공격때만 콤보가 적용이 됨
	}
	else
	{
		bComboAttackInput = true;
	}

	BowDrawPressed();

}

void AMain::LMBUp()
{
	bLMBDown = false;

	BowDrawReleased();
}

void AMain::RMBDown()
{
	bRMBDown = true;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	BowAimInputPressed();
}

void AMain::RMBUp()
{
	bRMBDown = false;

	BowAimInputReleased();
}

void AMain::TestKeyDown()
{
	bTestKeyDown = true;
}

void AMain::TestKeyUp()
{
	bTestKeyDown = false;
}

void AMain::EscDown()
{
	bEscDown = true;

	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}
}

void AMain::EscUp()
{
	bEscDown = false;
}

void AMain::SwordEquipKeyDown()
{
	bSwordEquipKeyDown = true;

	if (BowEquipped) return;

	AWeapon* Weapon = Cast<AWeapon>(SwordAttached->GetChildActor());
	if (Weapon && !GetMovementComponent()->IsFalling() && GetArmedStatus() == EArmedStatus::EAS_Normal)
	{
		bArmedBridgeIng = true;
		PlayMontage(WeaponEquipMontage, "SwordEquip");
	}
	else if (Weapon && !GetMovementComponent()->IsFalling() && GetArmedStatus() == EArmedStatus::EAS_Sword)
	{
		bArmedBridgeIng = true;
		PlayMontage(WeaponEquipMontage, "SwordUnEquip");
	}

}

void AMain::SwordEquipKeyUp()
{
	bSwordEquipKeyDown = false;
}

void AMain::ShowPickupLocations()
{
	for (auto Location : PickupLocations)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 16, FLinearColor::Green, 5.f, 0.5f);
	}
}

void AMain::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	EquippedWeapon = WeaponToSet;
}

void AMain::Equip()
{
	AWeapon* Weapon = Cast<AWeapon>(SwordAttached->GetChildActor());
	if (Weapon)
	{
		SetArmedStatus(EArmedStatus::EAS_Sword);
		Weapon->Equip(this);
	}
}

void AMain::UnEquip()
{
	AWeapon* Weapon = Cast<AWeapon>(SwordAttached->GetChildActor());
	if (Weapon)
	{
		SetArmedStatus(EArmedStatus::EAS_Normal);
		Weapon->UnEquip();
	}
}

void AMain::ArmedBridgeEnd()
{
	bArmedBridgeIng = false;
}

void AMain::Attack()
{
	bAttacking = true;
	if (CombatTarget) SetInterpToEnemy(true);	// 적이 나를 공격 대상으로 삼았다면 공격 보정이 가능한 상태로 만듦

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//애니메이션 인스턴스를 가져옴
	if (AnimInstance && CombatMontage)
	{
		FName ComboList[] = { "Combo01", "Combo02" };
		if (!(AnimInstance->Montage_IsPlaying(CombatMontage)))	//애니메이션이 실행 중이지 않을 때
		{
			AnimInstance->Montage_Play(CombatMontage, 1.0f);	// 해당 몽타주를 해당 속도로 실행
			AnimInstance->Montage_JumpToSection(FName(ComboList[ComboCnt]), CombatMontage);	//해당 섹션 실행
		}
		else {	//애니메이션이 실행 중 일 때
			AnimInstance->Montage_Play(CombatMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName(ComboList[ComboCnt]), CombatMontage);
		}
	}
}

void AMain::AirAttack() {

	bAttacking = true;

	int32 Temp = JumpCurrentCount;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);	// Flying 상태로 전환 , 중력 x
	JumpCurrentCount = Temp;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//애니메이션 인스턴스를 가져옴
	if (AnimInstance && JumpCombatMontage)
	{
		FName ComboList[] = { "JumpCombo01", "JumpCombo02", "JumpCombo03", "JumpCombo04", "JumpCombo05", "JumpCombo06", };
		if (!(AnimInstance->Montage_IsPlaying(JumpCombatMontage)))	//애니메이션이 실행 중이지 않을 때
		{
			AnimInstance->Montage_Play(JumpCombatMontage, 1.0f);	// 해당 몽타주를 해당 속도로 실행
			AnimInstance->Montage_JumpToSection(FName(ComboList[ComboCnt]), JumpCombatMontage);	//해당 섹션 실행
		}
		else {	//애니메이션이 실행 중 일 때
			AnimInstance->Montage_Play(JumpCombatMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName(ComboList[ComboCnt]), JumpCombatMontage);
		}
	}
}

void AMain::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);

	bComboAttackInput = false;
	ComboCnt = 0;

	if (GetCharacterMovement()->IsFlying())		// 공중 공격 중이었다면
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);	// Falling 상태로 전환, 중력 o 
	}
}

void AMain::ComboAttackInputCheck()
{
	if (bComboAttackInput)
	{
		bComboAttackInput = false;
		ComboCnt++;

		if (GetCharacterMovement()->IsFlying()) // 공중 공격 중이면
		{
			AirAttack();
		}
		else
		{
			Attack();
		}
	}
}

void AMain::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound)		// 무기를 휘둘렀을 때 나는 소리
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float HitDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (bSuperArmor) return HitDamage;

	FRotator LookAtRotation = GetLookAtRotationYaw(DamageCauser->GetActorLocation());

	FRotator HitRotation = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, GetActorRotation());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *HitRotation.ToString());

	if (GetMovementStatus() == EMovementStatus::EMS_Guard && -90.f < HitRotation.Yaw && HitRotation.Yaw < 90.f)	// 캐릭터가 가드 상태 일 시
	{
		SetGuardAccept(true);

		if (GuardAcceptSound)
		{
			UGameplayStatics::PlaySound2D(this, GuardAcceptSound);
		}
	}
	else //뒷쪽에서 공격 시, 가드 아닐 시 
	{
		if (!bGotHit)
		{
			if (DamageEvent.DamageTypeClass == KnockDown)
			{
				bGotHit = true;
				ResetCasting();
				PlayMontage(DamagedMontage, "KnockDown");

				FVector LaunchVelocity = GetActorUpVector() * -1500.f;
				LaunchCharacter(LaunchVelocity, false, false);
			}
			else if (DamageEvent.DamageTypeClass == Upper)
			{
				bGotHit = true;
				ResetCasting();
				PlayMontage(DamagedMontage, "Upper");

				SetActorRotation(LookAtRotation);
				FVector LaunchVelocity = GetActorUpVector() * 750.f;
				LaunchCharacter(LaunchVelocity, false, false);
			}
			else if (DamageEvent.DamageTypeClass == Rush)
			{
				bGotHit = true;
				ResetCasting();
				PlayMontage(DamagedMontage, "Rush");

				SetActorRotation(LookAtRotation);
				FVector LaunchVelocity = GetActorForwardVector() * -1500.f + GetActorUpVector() * 40.f;
				LaunchCharacter(LaunchVelocity, false, false);
			}
			else
			{
				UAnimInstance* Montage = GetMesh()->GetAnimInstance();

				int32 Num = FMath::RandRange(0, 1);
				FName MontageList[] = { "Damaged01", "Damaged02" };
				if (!Montage->GetCurrentActiveMontage() && !Aiming)
				{
					ResetCasting();
					PlayMontage(DamagedMontage, MontageList[Num]);
				}
			}

			DecrementHealth(HitDamage, DamageCauser);
			if (HitParticles) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, GetActorLocation(), FRotator(0.f), false);
			if (HitSound) UGameplayStatics::PlaySound2D(this, HitSound);
		}
	}

	return HitDamage;
}

void AMain::ResetCasting()	//무언가 동작 중에 다른 동작을 요구할 때
{
	if (bSkillCasting) SkillCastEnd();
	if (bArmedBridgeIng) ArmedBridgeEnd();
	if (bAttacking) {
		AttackEnd();
		AWeapon* Weapon = Cast<AWeapon>(SwordAttached->GetChildActor());
		if (Weapon)
		{
			Weapon->DeActivateCollision();
		}
	}
	if (Drawing) CanDraw = true;
	if (Aiming) ForceAimStop();

	UAnimInstance* Montage = GetMesh()->GetAnimInstance();
	if (Montage->GetCurrentActiveMontage() != DodgeMontage)
	{
		Montage->StopAllMontages(0.f);
	}

}

void AMain::DamagedEnd()
{
	bGotHit = false;
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);	// 목표 방향 Rotation을 반환
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

void AMain::UpdateCombatTarget()
{
	SetCombatTarget(nullptr);
	SetHasCombatTarget(false);
	if (MainPlayerController)
	{
		MainPlayerController->RemoveEnemyHealthBar();
	}

	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);	// 내가 겹쳐있는 모든 Actor의 정보를 가져옴, Enemy class의 정보만 필터

	if (OverlappingActors.Num() == 0) return;

	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	float MinDistance = TNumericLimits<float>::Max();

	for (auto Actor : OverlappingActors)	//제일 가까운 거리에 있는 Enemy의 정보와 거리 저장
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (Enemy)
		{
			float DistanceToActor = (Enemy->GetActorLocation() - GetActorLocation()).Size();
			if (MinDistance > DistanceToActor)
			{
				MinDistance = DistanceToActor;
				ClosestEnemy = Enemy;
			}
		}
	}

	if (ClosestEnemy)	// 공격시 해당 적을 향하도록 보간 및 적의 HP 표시
	{
		SetCombatTarget(ClosestEnemy);	// 메인에게 공격대상으로 설정했다고 알려주면서 enemy의 정보를 넘김
		SetHasCombatTarget(true);		// Enemy의 공격대상이니 Main에게 체력바를 보여주기 위함	

		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();
		}
	}
}

void AMain::GuardAcceptEnd()
{
	bGuardAccept = false;
}

void AMain::SwitchLevel(FName LevelName)
{
	SaveGame();

	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();							// output : UEDPIE_0_SunTemple
		CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);	// 앞에 붙은 접두사 제거
		UE_LOG(LogTemp, Warning, TEXT("MapName : %s, LevelName : %s"), *CurrentLevel, *LevelName.ToString());		// output : SunTemple

		FName CurrentLevelName(*CurrentLevel);	//FString 에서 FName 으로는 변환 가능, 반대는 불가능
		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMain::SaveGame()
{
	// 데이터를 설정할 비어 있는 새 SaveGame 개체를 만듦
	UFirstSaveGame* SaveGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	if (SaveGameInstance) // 데이터 저장
	{
		SaveGameInstance->CharacterStats.Health = Health;
		SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
		SaveGameInstance->CharacterStats.Stamina = Stamina;
		SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
		SaveGameInstance->CharacterStats.Coins = Coins;

		// Map Name 저장
		FString MapName = GetWorld()->GetMapName();					// output : UEDPIE_0_SunTemple
		MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);	// 앞에 붙은 접두사 제거
		//UE_LOG(LogTemp, Warning, TEXT("MapName : %s"), *MapName)	// output : SunTemple

		SaveGameInstance->CharacterStats.LevelName = MapName;

		if (EquippedWeapon)	// 장비된 무기의 이름 저장
		{
			SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
		}

		SaveGameInstance->CharacterStats.Location = GetActorLocation();
		SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

		// 해당 Name과 Index로 SaveGame 개체 저장
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
	}
}

void AMain::LoadGame(bool SetPosition)
{
	// 데이터를 설정할 비어 있는 새 SaveGame 개체를 만듦
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	if (LoadGameInstance)
	{
		// 해당 Name과 Index로 저장된 SaveGame 불러옴
		LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));
		if (LoadGameInstance) // 데이터 불러옴
		{
			Health = LoadGameInstance->CharacterStats.Health;
			MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
			Stamina = LoadGameInstance->CharacterStats.Stamina;
			MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
			Coins = LoadGameInstance->CharacterStats.Coins;

			// 이렇게 하는 이유는 무기는 포인터의 형태이기 때문에 그대로 저장하게되면 메모리가 저장되기때문에 날라가버림
			if (WeaponStorage)	// BP가 존재한다면
			{
				AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);	// BP에서 WeaponStorage가 선택된 것을 가져옴
				if (Weapons)	// 선택된 것이 해당 액터형태로 존재한다면 
				{
					FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;	// 장착했던 무기의 이름을 가져와서
					if (Weapons->WeaponMap.Contains(WeaponName))	// 해당 무기가 BP에서 설정되어 있다면, 맵에 존재한다면
					{
						AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);	// 해당 무기의 이름에 해당하는 값인 BP를 가져와서
						WeaponToEquip->Equip(this);		// 장착
					}
				}
			}

			// 초기값이 아니라면 해당 레벨로 이동
			if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
			{
				//UE_LOG(LogTemp, Warning, TEXT("MapName : %s"), *LoadGameInstance->CharacterStats.LevelName)		// output : SunTemple
				FName LevelName(*LoadGameInstance->CharacterStats.LevelName);

				SwitchLevel(LevelName);
			}

			//@@@@@FIX TODO :: 맵 바뀌면 setposition 풀리고 , 바뀐후 저장하면 true되게 해야됨
			if (SetPosition) // 새로운 레벨로 전환 할 때마다 게임을 로드해야함, 레벨이 바뀌면 전 레벨에서 저장한 위치는 의미가 없어짐
			{
				SetActorLocation(LoadGameInstance->CharacterStats.Location);
				SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
				//UE_LOG(LogTemp, Warning, TEXT("SetPosition Test"))
			}

			// 정상적인 상태로 되돌림
			Resurrection();
		}
	}
}

void AMain::LoadGameNoSwitch()
{
	// 데이터를 설정할 비어 있는 새 SaveGame 개체를 만듦
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	if (LoadGameInstance)
	{
		// 해당 Name과 Index로 저장된 SaveGame 불러옴
		LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));
		if (LoadGameInstance) // 데이터 불러옴
		{
			Health = LoadGameInstance->CharacterStats.Health;
			MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
			Stamina = LoadGameInstance->CharacterStats.Stamina;
			MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
			Coins = LoadGameInstance->CharacterStats.Coins;

			// 이렇게 하는 이유는 무기는 포인터의 형태이기 때문에 그대로 저장하게되면 메모리가 저장되기때문에 날라가버림
			if (WeaponStorage)	// BP가 존재한다면
			{
				AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);	// BP에서 WeaponStorage가 선택된 것을 가져옴
				if (Weapons)	// 선택된 것이 해당 액터형태로 존재한다면 
				{
					FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;	// 장착했던 무기의 이름을 가져와서
					if (Weapons->WeaponMap.Contains(WeaponName))	// 해당 무기가 BP에서 설정되어 있다면, 맵에 존재한다면
					{
						AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);	// 해당 무기의 이름에 해당하는 값인 BP를 가져와서
						WeaponToEquip->Equip(this);		// 장착
					}
				}
			}

			// 정상적인 상태로 만듦
			Resurrection();
		}
	}
}

void AMain::FirstSkillKeyDown()
{
	bFirstSkillKeyDown = true;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (EquippedWeapon && CanAction() && !FirstSkillCoolDown)
	{
		FireBallSkillCast();
	}

	if (BowEquipped && !BowReference->RainOfArrowsCoolDown)
	{
		if (BowReference->RainOfArrowsActive)
			BowReference->SetRainOfArrowsActive(false);
		else
			BowReference->SetRainOfArrowsActive(true);
	}
}

void AMain::FirstSkillKeyUp()
{
	bFirstSkillKeyDown = false;
}

void AMain::SecondSkillKeyDown()
{
	bSecondSkillKeyDown = true;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (EquippedWeapon && CanAction() && !SecondSkillCoolDown)
	{
		WaveSkillCast();
	}

	if (BowEquipped && !BowReference->IceArrowCoolDown)
	{
		if (BowReference->IceArrowActive)
			BowReference->SetIceArrowActive(false);
		else
			BowReference->SetIceArrowActive(true);
	}
}

void AMain::SecondSkillKeyUp()
{
	bSecondSkillKeyDown = false;
}

void AMain::ThirdSkillKeyDown()
{
	bThirdSkillKeyDown = true;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (EquippedWeapon && CanAction() && !ThirdSkillCoolDown)
	{
		Blackhole();
	}

	if (BowEquipped && !BowReference->DetectionArrowCoolDown)
	{
		if (BowReference->DetectionArrowActive)
			BowReference->SetDetectionArrowActive(false);
		else
			BowReference->SetDetectionArrowActive(true);
	}
}

void AMain::ThirdSkillKeyUp()
{
	bThirdSkillKeyDown = false;
}

void AMain::FourthSkillKeyDown()
{
	bFourthSkillKeyDown = true;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (EquippedWeapon && CanAction() && !FourthSkillCoolDown)
	{
		CloneAttack();
	}
}

void AMain::FourthSkillKeyUp()
{
	bFourthSkillKeyDown = false;
}

void AMain::PlayMontage(UAnimMontage* Montage, FName Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//애니메이션 인스턴스를 가져옴
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage, 1.0f);	// 해당 몽타주를 해당 속도로 실행
		AnimInstance->Montage_JumpToSection(Section, Montage);	//해당 섹션 실행
	}
}

void AMain::FireBallSkillCast()
{
	bSkillCasting = true;
	
	PlayMontage(SkillMontage, "Skill01");

	StartFirstSkillCooldown();
}

void AMain::WaveSkillCast()
{
	bSkillCasting = true;

	PlayMontage(SkillMontage, "Skill02");

}

void AMain::SkillCastEnd()
{
	bSkillCasting = false;
}

void AMain::FireBallSkillActivation()
{
	if (FireBall)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		AShootingSkill* ShootingSkill = GetWorld()->SpawnActor<AShootingSkill>(FireBall, GetActorLocation(), GetActorRotation(), SpawnParams);
		ShootingSkill->SetInstigator(GetController());
	}
}

void AMain::PlayFireBallSkillParticle()
{
	FVector ActorFloorLocation = GetActorLocation();
	ActorFloorLocation.Z -= 100.f;

	if (FireBallSkillParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireBallSkillParticle, ActorFloorLocation, GetActorRotation(), true);
	}
}

void AMain::PlayFireBallSkillSound()
{
	if (FireBallSkillSound)
	{
		UGameplayStatics::PlaySound2D(this, FireBallSkillSound);
	}
}

void AMain::WaveSkillActivation()
{
	FVector ActorLoc = GetActorLocation();
	FVector ActorForward = GetActorForwardVector();
	FVector StartLoc = ActorLoc; // 레이저 시작 지점.
	FVector EndLoc = ActorLoc + (ActorForward * 1000.0f); // 레이저 끝나는 지점.

	float Radius = 100.f;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes; // 히트 가능한 오브젝트 유형들.
	TEnumAsByte<EObjectTypeQuery> Target = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2);
	ObjectTypes.Add(Target);	//Enemy

	TArray<AActor*> IgnoreActors; // 무시할 액터들.

	TArray<FHitResult> HitResults; // 히트 결과 값 받을 변수.

	bool Result = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		StartLoc,
		EndLoc,
		Radius,
		ObjectTypes,
		false,
		IgnoreActors, // 무시할 것이 없다고해도 null을 넣을 수 없다.
		EDrawDebugTrace::None,	// 디버그
		HitResults,
		true
		// 여기 밑에 3개는 기본 값으로 제공됨. 바꾸려면 적으면 됨.
		//, FLinearColor::Red
		//, FLinearColor::Green
		//, 5.0f
	);

	if (Result == true)
	{
		// FVector ImpactPoint = HitResult.ImpactPoint;
		// HitResult에서 필요한 값들을 사용하면 됨.
		for (auto HitResult : HitResults) {
			AEnemy* Enemy = Cast<AEnemy>(HitResult.Actor);
			if (Enemy)
			{
				if (Enemy->HitParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, HitResult.ImpactPoint, FRotator(0.f), true);
				}
				if (Enemy->HitSound)
				{
					UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
				}
				if (Basic)
				{
					FName SectionName = GetMesh()->GetAnimInstance()->Montage_GetCurrentSection();

					UGameplayStatics::ApplyPointDamage(Enemy, WaveSkillDamage, GetActorLocation(), HitResult, MainInstigator, this, Basic);
					Enemy->GetCharacterMovement()->MaxWalkSpeed = 100.f;
					Enemy->bSlowDebuff = true;
				}
			}
		}
	}
}

void AMain::PlayWaveSkillParticle()
{

	if (WaveSkillParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WaveSkillParticle, GetActorLocation(), GetActorRotation(), true);
	}
}

void AMain::PlayWaveSkillSound()
{
	if (WaveSkillSound)
	{
		UGameplayStatics::PlaySound2D(this, WaveSkillSound);
	}
}

void AMain::SkillKeyDownCheck()
{
	if (!bSecondSkillKeyDown)
	{
		PlayMontage(SkillMontage, "Skill02End");

		StartSecondSkillCooldown();
	}
}

void AMain::DashAttack()
{
	bAttacking = true;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//애니메이션 인스턴스를 가져옴
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);	// 해당 몽타주를 해당 속도로 실행
		AnimInstance->Montage_JumpToSection("DashAttack", CombatMontage);	//해당 섹션 실행
	}
}

void AMain::UpperAttack()
{
	bAttacking = true;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//애니메이션 인스턴스를 가져옴
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);	// 해당 몽타주를 해당 속도로 실행
		AnimInstance->Montage_JumpToSection("UpperAttack", CombatMontage);	//해당 섹션 실행
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			FVector LaunchVelocity = GetActorUpVector() * 650.f;
			LaunchCharacter(LaunchVelocity, false, false);

			// TimerHandle 초기화
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}), 0.5f, false);	// 반복하려면 false를 true로 변경
}

FVector AMain::GetFloor()
{
	FVector ActorLoc = GetActorLocation();
	FVector ActorUp = GetActorUpVector();
	FVector StartLoc = ActorLoc; // 레이저 시작 지점.
	FVector EndLoc = ActorLoc + (ActorUp * -1000.0f); // 레이저 끝나는 지점.

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes; // 히트 가능한 오브젝트 유형들.
	TEnumAsByte<EObjectTypeQuery> WorldStatic = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic);
	ObjectTypes.Add(WorldStatic);

	TArray<AActor*> IgnoreActors; // 무시할 액터들.

	FHitResult HitResult; // 히트 결과 값 받을 변수.

	bool Result = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		StartLoc,
		EndLoc,
		ObjectTypes,
		false,
		IgnoreActors, // 무시할 것이 없다고해도 null을 넣을 수 없다.
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
		return HitResult.ImpactPoint;
	}

	return FVector(0.f);
}

void AMain::DodgeStart()
{
	bDodgeIng = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);	// Enemy에 대한 충돌 설정
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);	// Enemy의 무기
}

void AMain::DodgeEnd()
{
	bDodgeIng = false;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);	// Enemy에 대한 충돌 설정
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);	// Enemy의 무기

	if (bGotHit) bGotHit = false;
	CanDraw = true;
}

void AMain::AddInstanceQuiverArrows()
{
	FTransform Transform;
	Transform.SetRotation(FQuat(FRotator(-39.f, 167.f, 52.f)));
	Transform.SetScale3D(FVector(0.95f, 1.8f, 1.8f));
	for(int i=0; i<8; i++) QuiverArrows->AddInstance(Transform);
}

void AMain::BowInitSet()
{
	BowReference = Cast<ABow>(BowAttached->GetChildActor());
	if (BowReference == nullptr) return;

	BowReference->BeginPlayBow(this);
		
	ArrowMeshesInquiver = QuiverArrows->GetInstanceCount() - 1;
	SaveArrowTransforms();

	//StartTransform = GetActorTransform();
}

void AMain::SaveArrowTransforms()
{
	for (int i = 0; i <= ArrowMeshesInquiver; i++)
	{
		FTransform OutInstanceTransform;
		QuiverArrows->GetInstanceTransform(i, OutInstanceTransform);
		ArrowTransform.Emplace(OutInstanceTransform);
	}

}

void AMain::BowEquipKeyDown()
{
	bBowEquipKeyDown = true;

	if (EquippedWeapon) return;

	if (!BowEquipped)
	{
		bArmedBridgeIng = true;
		PlayMontage(BowEquipMontage, "BowEquip");
	}
	else
	{
		bArmedBridgeIng = true;
		PlayMontage(BowEquipMontage, "BowUnEquip");
	}
}

void AMain::BowEquipKeyUp()
{
	bBowEquipKeyDown = false;
}

void AMain::EquipBow()
{
	BowAttached->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true), "Bow1_Socket");
	if (BowReference->BowEquipSound) UGameplayStatics::PlaySound2D(this, BowReference->BowEquipSound);

	BowEquipped = true;
	SetArmedStatus(EArmedStatus::EAS_Bow);

	BowReference->EquippedChanged(false);

	if (Arrows > 0) RemoveArrowMeshQuiver(Arrows - 1);

	if (WantsToAim) CheckWantsToAim();

}

void AMain::UnEquipBow()
{
	BowReference->HideArrow();
	BowReference->DontHoldCable();

	if (Aiming) ForceAimStop();

	BowEquipped = false;
	SetArmedStatus(EArmedStatus::EAS_Normal);

	BowAttached->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true), "Bow_Back");
	if (BowReference->BowUnEquipSound) UGameplayStatics::PlaySound2D(this, BowReference->BowUnEquipSound);

	BowReference->EquippedChanged(true);

	if (Arrows > 0) SubtractNumber = 1;
	AddArrowMeshQuiver(Arrows);
}

void AMain::RemoveArrowMeshQuiver(int32 NewArrows)
{
	QuiverArrows->RemoveInstance(NewArrows);
}

void AMain::AddArrowMeshQuiver(int32 NewArrows)
{
	if (NewArrows - SubtractNumber < ArrowMeshesInquiver + 1)
	{
		int32 idx = QuiverArrows->GetInstanceCount();
		QuiverArrows->AddInstance(ArrowTransform[idx]);
	}
}


void AMain::SubtractArrows(int32 ArrowsToSubtract)	// 보통 ArrowsToSubtract = 1
{
	if (Arrows <= 0) return;

	Arrows -= ArrowsToSubtract;

	if (Arrows == 0) BowReference->HideArrow();

	BowReference->UpdateAmmoUI(Arrows);
}

void AMain::AddArrows(int32 ArrowsToAdd)	// 보통 ArrowsToAdd = 1
{
	int32 CurrentAmmo = Arrows;

	if (Arrows >= MaxArrows) return;

	Arrows += ArrowsToAdd;

	if (Aiming && (CurrentAmmo == 0))
	{
		SubtractNumber = 2;
	}
	else
	{
		for (int i = 0; i <= ArrowsToAdd - 1; i++)
		{
			AddArrowMeshQuiver(CurrentAmmo + 1);
			if (QuiverArrows->GetInstanceCount() == ArrowMeshesInquiver) SubtractNumber = 1;
		}
	}

	BowReference->UpdateAmmoUI(Arrows);

	if (CurrentAmmo == 0)
		if (Aiming && BowEquipped) 
			BowReference->ShowArrow();
}

void AMain::BowAimInputPressed()
{
	WantsToAim = true;

	ForceAimStart();
}

void AMain::BowAimInputReleased()
{
	WantsToAim = false;

	ForceAimStop();
}

void AMain::CheckWantsToAim()
{
	if (WantsToAim) ForceAimStart();
}

void AMain::ForceAimStart()
{
	if (!BowEquipped || bGotHit) return;

	SetMovementStatus(EMovementStatus::EMS_Normal);

	BowReference->AimStart();

	StartAim();

	if (Arrows > 0) BowReference->ShowArrow();

	CheckWantsToDraw();
}

void AMain::ForceAimStop()
{
	if (!BowEquipped) return;

	BowReference->AimStop();

	StopAim();
}

void AMain::StartAim()
{
	Aiming = true;
	
	ChangeFollowCameraZoomIn();
}

void AMain::StopAim()
{
	Aiming = false;
	
	ChangeFollowCamera();
}

void AMain::WalkingWayChange()
{
	if (bWalking8Way) bWalking8Way = false;
	else bWalking8Way = true;
}

void AMain::ChangeFollowCameraZoomOut()
{
	bCameraZoomOut = true;
	CameraBoom->bEnableCameraLag = false;
}

void AMain::ChangeFollowCameraZoomIn()
{
	bCameraZoomIn = true;
	CameraBoom->bEnableCameraLag = false;
}

void AMain::ChangeFollowCamera()
{
	bCameraZoomOut = false;
	bCameraZoomIn = false;
	CameraBoom->bEnableCameraLag = true;
}

void AMain::CameraChangeInterp()
{
	if (bCameraZoomIn) 
	{
		FVector NewLocation = UKismetMathLibrary::VInterpTo(FollowCamera->GetRelativeLocation(), FollowCameraZoomIn->GetRelativeLocation(), GetWorld()->GetDeltaSeconds(), 5.f);
		FollowCamera->SetRelativeLocation(NewLocation);
	}
	else if (bCameraZoomOut)
	{
		FVector NewLocation = UKismetMathLibrary::VInterpTo(FollowCamera->GetRelativeLocation(), FollowCameraInitLoc + FVector(-500.f, 0.f, 200.f), GetWorld()->GetDeltaSeconds(), 5.f);
		FollowCamera->SetRelativeLocation(NewLocation);
	}
	else
	{
		FVector NewLocation = UKismetMathLibrary::VInterpTo(FollowCamera->GetRelativeLocation(), FollowCameraInitLoc, GetWorld()->GetDeltaSeconds(), 5.f);
		FollowCamera->SetRelativeLocation(NewLocation);
	}
}

void AMain::BowDrawPressed()
{
	WantsToDraw = true;

	if (CanDraw && BowEquipped && Aiming) CheckDrawStart();
}

void AMain::BowDrawReleased()
{
	WantsToDraw = false;

	if (!(CanDraw && BowEquipped && Aiming && Arrows > 0)) return;

	BowReference->DrawShoot();
	Drawing = false;
	PlayFireMontage();
}

void AMain::CheckWantsToDraw()
{
	if (WantsToDraw && CanDraw && BowEquipped) CheckDrawStart();
}

void AMain::CheckDrawStart()
{
	if (Arrows > 0) BowReference->DrawStart();
	Drawing = true;
}

void AMain::PlayFireMontage()
{
	BowReference->HideArrow();
	BowReference->DontHoldCable();

	if (ArrowFireMontage) PlayMontage(ArrowFireMontage, "Default");

	// Temporarily don't allow draw
	CanDraw = false;
}

void AMain::FireArrowEnd()
{
	CanDraw = true;
	
	if (WantsToDraw) CheckWantsToDraw();

	if (Arrows != 0) BowReference->ShowArrow();
	BowReference->HoldCable();
}

void AMain::LockOnTargetKeyDown()
{
	bLockOnTargetKeyDown = true;

	LockOnTarget();
}

void AMain::LockOnTargetKeyUp()
{
	bLockOnTargetKeyDown = false;
}

void AMain::LockOnTarget()
{
	if (bWalking8Way)
	{
		ChangeFollowCamera();

		LockOnEnemy = nullptr;
		bWalking8Way = false;
		bUseControllerRotationYaw = false;
	}
	else {

		FVector ActorLoc = GetActorLocation();
		FVector CamForward = FollowCamera->GetForwardVector();
		FVector StartLoc = ActorLoc; 
		FVector EndLoc = ActorLoc + (CamForward * 2500.0f); 

		float Radius = 300.f;

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		TEnumAsByte<EObjectTypeQuery> Target = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2);
		ObjectTypes.Add(Target);	//Enemy

		TArray<AActor*> IgnoreActors;
		FHitResult HitResult;

		bool Result = UKismetSystemLibrary::SphereTraceSingleForObjects(
			GetWorld(),
			StartLoc,
			EndLoc,
			Radius,
			ObjectTypes,
			false,
			IgnoreActors,
			EDrawDebugTrace::None,	
			HitResult,
			true
			// 여기 밑에 3개는 기본 값으로 제공됨. 바꾸려면 적으면 됨.
			//, FLinearColor::Red
			//, FLinearColor::Green
			//, 5.0f
		);

		if (Result)
		{
			LockOnEnemy = Cast<AEnemy>(HitResult.GetActor());
			if (LockOnEnemy)
			{
				ChangeFollowCameraZoomIn();

				bWalking8Way = true;

				// Lock Character Move Forward
				bUseControllerRotationYaw = true;
			}
		}
	}
}

void AMain::LockOnTargetCameraInterp()
{
	if (!bWalking8Way && !LockOnEnemy) return;

	FRotator NewRotation;
	NewRotation.Roll = GetController()->GetControlRotation().Roll;

	FVector Target = LockOnEnemy->GetActorLocation();
	Target.Z -= 100.f;
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);

	FRotator InterpRotation = FMath::RInterpTo(GetController()->GetControlRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), 5.f);
	
	NewRotation.Pitch = InterpRotation.Pitch;
	NewRotation.Yaw = InterpRotation.Yaw;
	
	GetController()->SetControlRotation(NewRotation);
}

void AMain::SpawnDamageNumbers(FVector InLocation, bool Headshot, float DamageNumbers)
{
	if (DamageNumberAsset)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(InLocation);
		ADamageNumbers* DamageNum = GetWorld()->SpawnActorDeferred<ADamageNumbers>(DamageNumberAsset, SpawnTransform);
		
		FString Str = FString::FromInt(FGenericPlatformMath::TruncToInt(DamageNumbers));
		DamageNum->DamageNumbers = FText::FromString(Str);
		DamageNum->Headshot = Headshot;

		DamageNum->FinishSpawning(SpawnTransform);
	}
}

void AMain::Combo1KeyDown()
{
	bCombo1KeyDown = true;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (EquippedWeapon && CanAction())
	{
		DashAttack();
	}
}

void AMain::Combo1KeyUp()
{
	bCombo1KeyDown = false;
}

void AMain::Combo2KeyDown()
{
	bCombo2KeyDown = true;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (EquippedWeapon && CanAction())
	{
		UpperAttack();
	}
}

void AMain::Combo2KeyUp()
{
	bCombo2KeyDown = false;
}

void AMain::Blackhole()
{
	if (bBlackholeKeyDown)
	{
		bBlackholeKeyDown = false;
		BlackholeRangeDecal->Destroy();
		BlackholeRangeDecal = nullptr;

		ChangeFollowCamera();
	}
	else 
	{
		bBlackholeKeyDown = true;
		if (BlackholeRangeDecalAsset)
		{
			FTransform Transform;
			Transform.SetScale3D(FVector(4.f));
			BlackholeRangeDecal = GetWorld()->SpawnActor<APositionDecal>(BlackholeRangeDecalAsset, Transform);

			ChangeFollowCameraZoomOut();
		}
	}
}

void AMain::BlackholeSKillCast()
{
	bBlackholeKeyDown = false;
	
	bSkillCasting = true;
	bBlackholeActive = true;
	
	ChangeFollowCamera();

	StartThirdSkillCooldown();

	BlackholeRangeDecal->Destroy();
	BlackholeRangeDecal = nullptr;

	FVector ActorLoc = MousePoint;
	ActorLoc.Z += 100.f;
	SetActorLocation(ActorLoc, false, nullptr, ETeleportType::TeleportPhysics);
	GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(CamShake);

	PlayMontage(SkillMontage, "Skill03");
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	bSuperArmor = true;

	if (BlackholeStartParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BlackholeStartParticle, MousePoint);
	if (BlackholeStartSound) UGameplayStatics::PlaySound2D(this, BlackholeStartSound);
}

void AMain::ShowBlackholeRange()
{
	if (!bBlackholeKeyDown) return;

	HoldMouseCenter();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes; // 히트 가능한 오브젝트 유형들.
	TEnumAsByte<EObjectTypeQuery> WorldStatic = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic);
	ObjectTypes.Add(WorldStatic);	//WorldStatic
	bool Result = MainPlayerController->GetHitResultUnderCursorForObjects(ObjectTypes, true, MouseHitResult);

	if (!Result) return;

	MousePoint = MouseHitResult.ImpactPoint;
	float Dist = UKismetMathLibrary::Vector_Distance(GetActorLocation(), MousePoint);
	if (Dist > 2000.f) 
		MousePoint = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 2500.f);

	BlackholeRangeDecal->SetActorLocation(MousePoint, false, nullptr, ETeleportType::TeleportPhysics);
}

void AMain::HoldMouseCenter()
{
	if (!MainPlayerController) return;

	int32 X, Y;
	MainPlayerController->GetViewportSize(X, Y);
	X /= 2;
	Y /= 2;
	MainPlayerController->SetMouseLocation(X, Y);
}

void AMain::BlackholeCastEnd()
{
	bSkillCasting = false;
	
	bBlackholeActive = false;
	bSuperArmor = false;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

	if (BlackholeEndParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BlackholeEndParticle, MousePoint, FRotator(0.f), FVector(3.f));
	if (BlackholeEndSound) UGameplayStatics::PlaySound2D(this, BlackholeEndSound);

	for (auto HitResult : BlackholeHitResults)
	{
		AEnemy* Enemy = Cast<AEnemy>(HitResult.GetActor());
		if (Enemy)
		{
			Enemy->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
			UGameplayStatics::ApplyPointDamage(Enemy, 99.f, GetActorLocation(), HitResult, MainInstigator, this, Rush);
		}
	}
	BlackholeHitResults.Empty();
}

void AMain::PlayBlackholeParticle()
{
	if (BlackholeParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BlackholeParticle, MousePoint);
	if (BlackholeSound) UGameplayStatics::PlaySound2D(this, BlackholeSound);
}

void AMain::CloneAttack()
{
	if (bCloneAttackKeyDown) return;

	bCloneAttackKeyDown = true;

	StartFourthSkillCooldown();

	FTimerHandle CloneAttackKeyDownHandle;
	GetWorld()->GetTimerManager().SetTimer(CloneAttackKeyDownHandle, FTimerDelegate::CreateLambda([&]()
		{
			bCloneAttackKeyDown = false;
			bCloneReset = true;
		}), 7.f, false);
}

void AMain::CloneAttackCast()
{
	if (bCloneAttackKeyDown && !CloneAttackDelay)
	{
		CloneAttackDelay = true;
		GetWorld()->GetTimerManager().SetTimer(CloneAttackHandle, FTimerDelegate::CreateLambda([&]()
			{
				TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
				TEnumAsByte<EObjectTypeQuery> Target = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2);
				ObjectTypes.Add(Target);	//Enemy

				TArray<AActor*> IgnoreActors;
				FHitResult HitResult;

				bool Result = UKismetSystemLibrary::SphereTraceSingleForObjects(
					GetWorld(),
					GetActorLocation(),
					GetActorLocation(),
					800.f,
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

				if (Result)
				{
					// Make Clone
					FTransform SpawnTransform;

					AMain* Clone = GetWorld()->SpawnActorDeferred<AMain>(CloneAsset, SpawnTransform);
					
					Clone->SetOwner(this);
					Clone->SetInstigator(GetController());
					Clone->FinishSpawning(SpawnTransform);

					Clone->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
					Clone->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);
					Clone->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
					Clone->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
					Clone->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);
					Clone->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
					Clone->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
					
					Clones.Emplace(Clone);
					
					FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(HitResult.ImpactPoint, FVector(300.f, 300.f, 0.f));
					Clone->SetActorLocation(Point);
					FVector CloneFloor = Clone->GetFloor();
					Point.Z = CloneFloor.Z + 100.f;
					Clone->SetActorLocation(Point);

					FVector Start = Clone->GetActorLocation();
					FVector LookTarget = HitResult.GetActor()->GetActorLocation();
					FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Start, LookTarget);
					LookAtRotation = FRotator(0.f, LookAtRotation.Yaw, 0.f);

					Clone->SetActorRotation(LookAtRotation);

					if (CloneSpawnParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CloneSpawnParticle, Point);
					if (CloneSpawnSound) UGameplayStatics::PlaySoundAtLocation(this, CloneSpawnSound, Point);

					// Make Weapon
					AWeapon* Sword = GetWorld()->SpawnActor<AWeapon>(WeaponAsset);
					CloneWeapons.Emplace(Sword);
					Sword->Equip(Clone);
					Sword->SetInstigator(GetController());

					TArray<FName> ComboArr = { "Combo01", "Combo02", "DashAttack", "UpperAttack" };
					int32 RandNum = FMath::RandRange(0, 3);

					Clone->PlayMontage(CombatMontage, ComboArr[RandNum]);
				}


				CloneAttackDelay = false;
			}), 1.f, false);
	}
	else if (!bCloneAttackKeyDown && !CloneAttackDelay)
	{
		if (bCloneReset)
		{
			bCloneReset = false;

			for (auto Clone : Clones)
			{
				if (CloneRemoveParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CloneRemoveParticle, Clone->GetActorLocation());
				if (CloneRemoveSound) UGameplayStatics::PlaySoundAtLocation(this, CloneRemoveSound, Clone->GetActorLocation());

				Clone->Destroy();
			}
			for (auto Weapon : CloneWeapons) Weapon->Destroy();
			Clones.Empty();
			CloneWeapons.Empty();
		}
	}
}

void AMain::StartFirstSkillCooldown()
{
	FirstSkillCoolDown = true;

	FirstSkillCoolDownUI = 0.f;

	FTimerHandle CoolDownHandle;
	GetWorld()->GetTimerManager().SetTimer(CoolDownHandle, FTimerDelegate::CreateLambda([&]()
		{
			FirstSkillCoolDown = false;
		}), FirstSkillCoolDownTime, false);
}

void AMain::StartSecondSkillCooldown()
{
	SecondSkillCoolDown = true;

	SecondSkillCoolDownUI = 0.f;

	FTimerHandle CoolDownHandle;
	GetWorld()->GetTimerManager().SetTimer(CoolDownHandle, FTimerDelegate::CreateLambda([&]()
		{
			SecondSkillCoolDown = false;
		}), SecondSkillCoolDownTime, false);
}

void AMain::StartThirdSkillCooldown()
{
	ThirdSkillCoolDown = true;

	ThirdSkillCoolDownUI = 0.f;

	FTimerHandle CoolDownHandle;
	GetWorld()->GetTimerManager().SetTimer(CoolDownHandle, FTimerDelegate::CreateLambda([&]()
		{
			ThirdSkillCoolDown = false;
		}), ThirdSkillCoolDownTime, false);
}

void AMain::StartFourthSkillCooldown()
{
	FourthSkillCoolDown = true;

	FourthSkillCoolDownUI = 0.f;

	FTimerHandle CoolDownHandle;
	GetWorld()->GetTimerManager().SetTimer(CoolDownHandle, FTimerDelegate::CreateLambda([&]()
		{
			FourthSkillCoolDown = false;
		}), FourthSkillCoolDownTime, false);
}

void AMain::FirstSkillCoolDownReturn()
{
	if (FirstSkillCoolDown && !FirstSkillCoolDownDelay)
	{
		FirstSkillCoolDownDelay = true;
		FTimerHandle CoolDownHandle;
		GetWorld()->GetTimerManager().SetTimer(CoolDownHandle, FTimerDelegate::CreateLambda([&]()
			{
				FirstSkillCoolDownUI += 1.f / FirstSkillCoolDownTime;
				FirstSkillCoolDownDelay = false;
			}), 1.f, false);
	}
}

void AMain::SecondSkillCoolDownReturn()
{
	if (SecondSkillCoolDown && !SecondSkillCoolDownDelay)
	{
		SecondSkillCoolDownDelay = true;
		FTimerHandle CoolDownHandle;
		GetWorld()->GetTimerManager().SetTimer(CoolDownHandle, FTimerDelegate::CreateLambda([&]()
			{
				SecondSkillCoolDownUI += 1.f / SecondSkillCoolDownTime;
				SecondSkillCoolDownDelay = false;
			}), 1.f, false);
	}
}

void AMain::ThirdSkillCoolDownReturn()
{
	if (ThirdSkillCoolDown && !ThirdSkillCoolDownDelay)
	{
		ThirdSkillCoolDownDelay = true;
		FTimerHandle CoolDownHandle;
		GetWorld()->GetTimerManager().SetTimer(CoolDownHandle, FTimerDelegate::CreateLambda([&]()
			{
				ThirdSkillCoolDownUI += 1.f / ThirdSkillCoolDownTime;
				ThirdSkillCoolDownDelay = false;
			}), 1.f, false);
	}
}

void AMain::FourthSkillCoolDownReturn()
{
	if (FourthSkillCoolDown && !FourthSkillCoolDownDelay)
	{
		FourthSkillCoolDownDelay = true;
		FTimerHandle CoolDownHandle;
		GetWorld()->GetTimerManager().SetTimer(CoolDownHandle, FTimerDelegate::CreateLambda([&]()
			{
				FourthSkillCoolDownUI += 1.f / FourthSkillCoolDownTime;
				FourthSkillCoolDownDelay = false;
			}), 1.f, false);
	}
}