// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MainAnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "FirstSaveGame.h"
#include "ItemStorage.h"
#include "Math/TransformNonVectorized.h"


// Sets default values
AMain::AMain()
{
	
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Camera Boom (pulls towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;	//Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true;		// Rotate arm based on controller ��Ʈ�ѷ� ������� ȸ��

	// Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(34.f, 88.f);

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);		// Attach the camera to the end of the boom and let the boom adjust to match 
	FollowCamera->bUsePawnControlRotation = false;		// �̹� springarm�� true�̱� ������ ��Ʈ�ѷ��� ������������

	// Set our turn rates for input , 1�ʵ��� Ű�� ������ ������ ȸ���ϴ� ��
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// ���̸� �� Pawn�� yaw�� PlayerController�� ���� ����Ǵ� ��� ��Ʈ�ѷ��� ControlRotation yaw�� ��ġ�ϵ��� ������Ʈ
	// Don't rotate when the controller rotates
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;	//Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f);	// ...at this rotation rate , yaw�� �ش��ϴ� �κ��� ���� �ø��� ȸ���ӵ��� ������
	GetCharacterMovement()->JumpZVelocity = 650.f;	// ���� ��
	GetCharacterMovement()->AirControl = 0.2f;	// ���� �� �����

	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 100.f;
	Stamina = 50.f;
	Coins = 0;

	RunningSpeed = 600.f;
	SprintingSpeed = 800.f;

	bShiftKeyDown = false;
	bInteractionKeyDown = false;
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
	bSkillCasting = false;

	//Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;
	//ArmedStatus = EArmedStatus::EAS_Normal;

	StaminaDrainRate = 20.f;
	MinSprintStamina = 30.f;

	InterpSpeed = 15.f;

	ComboCnt = 0;

	WaveSkillDamage = 25.f;

	//��Ʈ�ѷ��� �����ؾ� ���� ���ظ� �ִ��� �� �� �ְ� ���ظ� �� �� ����
	SetInstigator(GetController());

}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	FString Map = GetWorld()->GetMapName();
	Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	// ��� ������ �� �� ĳ������ ��Ʈ�ѷ��� �����ؾ� ���� ���ظ� �ִ��� �� �� �ְ� ���ظ� �� �� ����

	if (Map != "SunTemple")
	{
		LoadGameNoSwitch();
	}
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

	// ������ ������ �� �ֺ��� ���� �ִٸ� ������ȯ
	if (bInterpToEnemy && CombatTarget)		//Interp Rotation
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);	// �������ϰ� ȸ��

		SetActorRotation(InterpRotation);
	}

	// HP�� ���̱� ���� ��ġ ����
	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}

	// ���� ���� �ִٸ� Guard ���·� ��ȯ
	if (bRMBDown)
	{
		if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

		if (EquippedWeapon && Alive() && !bAttacking && !bSkillCasting)
		{
			SetMovementStatus(EMovementStatus::EMS_Guard);
		}
	}

}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent); // ��ȿ���� Ȯ�� �� false ��ȯ�ϸ� ���� ����

	//�ش��ϴ� Ű�� ������ �̿� �´� �Լ� ȣ��
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	//PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AMain::DodgeKeyDown);
	//PlayerInputComponent->BindAction("Dodge", IE_Released, this, &AMain::DodgeKeyUp);

	PlayerInputComponent->BindAction("Interaction", IE_Pressed, this, &AMain::InteractionKeyDown);
	PlayerInputComponent->BindAction("Interaction", IE_Released, this, &AMain::InteractionKeyUp);

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

	// MEMO : Tick ó�� ��� ���� �� .. ����?? , �Ʒ� �Լ��鵵 �������� �� �� ����
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
		!bAttacking && !bSkillCasting && Alive() &&
		!MainPlayerController->bPauseMenuVisible &&
		GetMovementStatus() != EMovementStatus::EMS_Guard;
}

bool AMain::CanAction()
{
	return Alive() && !bAttacking && !bSkillCasting && GetMovementStatus() != EMovementStatus::EMS_Guard;
}

void AMain::MoveForward(float Value)
{
	bMovingForward = false;		// �̷��� �ص� ������ �� true�� ���� �Ǵ� ��
	if (CanMove(Value))	// ���ݽ� ������ ����
	{
		bMovingForward = true;

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();		// ���� �� ���� ���⶧���� const  ,  ��Ʈ�ѷ��� ���ϴ� ������ �˷��ִ� rotation�� ��ȯ
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);		// �츮�� ������ ����鿡�� � ������ ���ϴ����� �߿�, �ϳ��� ���� �����´� ����

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);	// Matrix(Yam) = �ϳ��� ���� ������� �÷��̾��� local������ ������, Get(X) �� �߿��� X������ ������ = �÷��̾��� ����, ��
		AddMovementInput(Direction, Value);

	}
}

void AMain::MoveRight(float Value)
{
	bMovingRight = false;
	if (CanMove(Value))	// ���ݽ� ������ ����
	{
		bMovingRight = true;

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);	// X�� �����̶�� Y�� �¿� ����
		AddMovementInput(Direction, Value);

	}
}

void AMain::Jump()
{
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;	// �޴�â �������̶�� ����

	if (CanAction())
	{
		ACharacter::Jump();	// =  Super::Jump();
	}
}

void AMain::Turn(float Value)
{
	AddControllerYawInput(Value); // �÷��̾���Ʈ�ѷ��� Yaw���� ����
}

void AMain::LookUp(float Value)
{
	AddControllerPitchInput(Value); // �÷��̾���Ʈ�ѷ��� Pitch���� ����
}

void AMain::TurnAtRate(float Rate)
{
	//�Է��� �޾� �࿡�� ��Ʈ�ѷ��� ��ü������ ȸ����Ű�� ���
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());		// ƽ�Լ� �ܺο��� ��Ÿ�ð��� ��� ���
}

void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
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

bool AMain::Alive()	// ��������� true
{
	return GetMovementStatus() != EMovementStatus::EMS_Dead;
}

void AMain::Die()
{
	if (!Alive() && MovementStatus == EMovementStatus::EMS_Dead) return;

	SetMovementStatus(EMovementStatus::EMS_Dead);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//�ִϸ��̼� �ν��Ͻ��� ������
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
	GetMesh()->bPauseAnims = true;			// �ִϸ��̼� ����
	GetMesh()->bNoSkeletonUpdate = true;	// ���̷��� ������Ʈ ����
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
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
		Attack();
		bComboAttackInput = false;	//@@@@@ FIX TODO : �ּ�ó�� �� ù ���ݶ��� �޺��� ������ ��
	}
	else 
	{
		bComboAttackInput = true;
	}
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::RMBDown()
{
	bRMBDown = true;
}

void AMain::RMBUp()
{
	bRMBDown = false;
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

void AMain::InteractionKeyDown()
{
	bInteractionKeyDown = true;
	if (ActiveOverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			//SetArmedStatus(EArmedStatus::EAS_Sword);
			Weapon->Equip(this);
		}
	}
}

void AMain::InteractionKeyUp()
{
	bInteractionKeyDown = false;
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
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = WeaponToSet;
}

void AMain::Attack()
{
	bAttacking = true;
	if (CombatTarget) SetInterpToEnemy(true);	// ���� ���� ���� ������� ��Ҵٸ� ���� ������ ������ ���·� ����

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//�ִϸ��̼� �ν��Ͻ��� ������
	if (AnimInstance && CombatMontage)
	{
		const char* ComboList[] = { "Combo01", "Combo02" };
		if (!(AnimInstance->Montage_IsPlaying(CombatMontage)))	//�ִϸ��̼��� ���� ������ ���� ��
		{
			AnimInstance->Montage_Play(CombatMontage, 1.0f);	// �ش� ��Ÿ�ָ� �ش� �ӵ��� ����
			AnimInstance->Montage_JumpToSection(FName(ComboList[ComboCnt]), CombatMontage);	//�ش� ���� ����
		}
		else {	//�ִϸ��̼��� ���� �� �� ��
			AnimInstance->Montage_Play(CombatMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName(ComboList[ComboCnt]), CombatMontage);
		}
	}
}

void AMain::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);

	bComboAttackInput = false;
	ComboCnt = 0;
}

void AMain::ComboAttackInputCheck()
{
	if (bComboAttackInput)
	{
		bComboAttackInput = false;
		ComboCnt++;
		Attack();
	}
}

void AMain::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound)		// ���⸦ �ֵѷ��� �� ���� �Ҹ�
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
	DecrementHealth(DamageAmount, DamageCauser);

	return DamageAmount;
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);	// ��ǥ ���� Rotation�� ��ȯ
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
	GetOverlappingActors(OverlappingActors, EnemyFilter);	// ���� �����ִ� ��� Actor�� ������ ������, Enemy class�� ������ ����

	if (OverlappingActors.Num() == 0) return;

	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	float MinDistance = TNumericLimits<float>::Max();

	for (auto Actor : OverlappingActors)	//���� ����� �Ÿ��� �ִ� Enemy�� ������ �Ÿ� ����
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

	if (ClosestEnemy)	// ���ݽ� �ش� ���� ���ϵ��� ���� �� ���� HP ǥ��
	{
		SetCombatTarget(ClosestEnemy);	// ���ο��� ���ݴ������ �����ߴٰ� �˷��ָ鼭 enemy�� ������ �ѱ�
		SetHasCombatTarget(true);		// Enemy�� ���ݴ���̴� Main���� ü�¹ٸ� �����ֱ� ����	

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
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();							// output : UEDPIE_0_SunTemple
		CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);	// �տ� ���� ���λ� ����
		//UE_LOG(LogTemp, Warning, TEXT("MapName : %s"), *CurrentLevel)		// output : SunTemple

		FName CurrentLevelName(*CurrentLevel);	//FString ���� FName ���δ� ��ȯ ����, �ݴ�� �Ұ���
		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMain::SaveGame()
{
	// �����͸� ������ ��� �ִ� �� SaveGame ��ü�� ����
	UFirstSaveGame* SaveGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	if (SaveGameInstance) // ������ ����
	{
		SaveGameInstance->CharacterStats.Health = Health;
		SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
		SaveGameInstance->CharacterStats.Stamina = Stamina;
		SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
		SaveGameInstance->CharacterStats.Coins = Coins;

		// Map Name ����
		FString MapName = GetWorld()->GetMapName();					// output : UEDPIE_0_SunTemple
		MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);	// �տ� ���� ���λ� ����
		//UE_LOG(LogTemp, Warning, TEXT("MapName : %s"), *MapName)	// output : SunTemple

		SaveGameInstance->CharacterStats.LevelName = MapName;

		if (EquippedWeapon)	// ���� ������ �̸� ����
		{
			SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
		}

		SaveGameInstance->CharacterStats.Location = GetActorLocation();
		SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

		// �ش� Name�� Index�� SaveGame ��ü ����
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
	}
}

void AMain::LoadGame(bool SetPosition)
{
	// �����͸� ������ ��� �ִ� �� SaveGame ��ü�� ����
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	if (LoadGameInstance)
	{
		// �ش� Name�� Index�� ����� SaveGame �ҷ���
		LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));
		if (LoadGameInstance) // ������ �ҷ���
		{
			Health = LoadGameInstance->CharacterStats.Health;
			MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
			Stamina = LoadGameInstance->CharacterStats.Stamina;
			MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
			Coins = LoadGameInstance->CharacterStats.Coins;

			// �̷��� �ϴ� ������ ����� �������� �����̱� ������ �״�� �����ϰԵǸ� �޸𸮰� ����Ǳ⶧���� ���󰡹���
			if (WeaponStorage)	// BP�� �����Ѵٸ�
			{
				AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);	// BP���� WeaponStorage�� ���õ� ���� ������
				if (Weapons)	// ���õ� ���� �ش� �������·� �����Ѵٸ� 
				{
					FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;	// �����ߴ� ������ �̸��� �����ͼ�
					if (Weapons->WeaponMap.Contains(WeaponName))	// �ش� ���Ⱑ BP���� �����Ǿ� �ִٸ�, �ʿ� �����Ѵٸ�
					{
						AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);	// �ش� ������ �̸��� �ش��ϴ� ���� BP�� �����ͼ�
						WeaponToEquip->Equip(this);		// ����
					}
				}
			}

			// �ʱⰪ�� �ƴ϶�� �ش� ������ �̵�
			if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
			{
				//UE_LOG(LogTemp, Warning, TEXT("MapName : %s"), *LoadGameInstance->CharacterStats.LevelName)		// output : SunTemple
				FName LevelName(*LoadGameInstance->CharacterStats.LevelName);

				SwitchLevel(LevelName);
			}

			//@@@@@FIX TODO :: �� �ٲ�� setposition Ǯ���� , �ٲ��� �����ϸ� true�ǰ� �ؾߵ�
			if (SetPosition) // ���ο� ������ ��ȯ �� ������ ������ �ε��ؾ���, ������ �ٲ�� �� �������� ������ ��ġ�� �ǹ̰� ������
			{
				SetActorLocation(LoadGameInstance->CharacterStats.Location);
				SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
				//UE_LOG(LogTemp, Warning, TEXT("SetPosition Test"))
			}

			// �������� ���·� �ǵ���
			Resurrection();
		}
	}
}

//@@@@@FIX TODO : ����� ���� �״�� ���� ������ �Ѿ�� �� ����
void AMain::LoadGameNoSwitch()
{
	// �����͸� ������ ��� �ִ� �� SaveGame ��ü�� ����
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	if (LoadGameInstance)
	{
		// �ش� Name�� Index�� ����� SaveGame �ҷ���
		LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));
		if (LoadGameInstance) // ������ �ҷ���
		{
			Health = LoadGameInstance->CharacterStats.Health;
			MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
			Stamina = LoadGameInstance->CharacterStats.Stamina;
			MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
			Coins = LoadGameInstance->CharacterStats.Coins;

			// �̷��� �ϴ� ������ ����� �������� �����̱� ������ �״�� �����ϰԵǸ� �޸𸮰� ����Ǳ⶧���� ���󰡹���
			if (WeaponStorage)	// BP�� �����Ѵٸ�
			{
				AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);	// BP���� WeaponStorage�� ���õ� ���� ������
				if (Weapons)	// ���õ� ���� �ش� �������·� �����Ѵٸ� 
				{
					FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;	// �����ߴ� ������ �̸��� �����ͼ�
					if (Weapons->WeaponMap.Contains(WeaponName))	// �ش� ���Ⱑ BP���� �����Ǿ� �ִٸ�, �ʿ� �����Ѵٸ�
					{
						AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);	// �ش� ������ �̸��� �ش��ϴ� ���� BP�� �����ͼ�
						WeaponToEquip->Equip(this);		// ����
					}
				}
			}

			// �������� ���·� ����
			Resurrection();
		}
	}
}

void AMain::FirstSkillKeyDown()
{
	bFirstSkillKeyDown = true;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (EquippedWeapon && CanAction())
	{
		FireBallSkillCast();
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

	if (EquippedWeapon && CanAction())
	{
		WaveSkillCast();
	}
}

void AMain::SecondSkillKeyUp()
{
	bSecondSkillKeyDown = false;
}

void AMain::PlaySkillMontage(FName Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//�ִϸ��̼� �ν��Ͻ��� ������
	if (AnimInstance && SkillMontage)
	{
		AnimInstance->Montage_Play(SkillMontage, 1.0f);	// �ش� ��Ÿ�ָ� �ش� �ӵ��� ����
		AnimInstance->Montage_JumpToSection(Section, SkillMontage);	//�ش� ���� ����
	}
}

void AMain::FireBallSkillCast()
{
	bSkillCasting = true;

	PlaySkillMontage("Skill01");
}

void AMain::WaveSkillCast()
{
	bSkillCasting = true;

	PlaySkillMontage("Skill02");

}

void AMain::SkillCastEnd()
{
	bSkillCasting = false;
}

void AMain::FireBallSkillActivation()
{
	UWorld* World = GetWorld();
	
	if (World && FireBall)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		FRotator Rotator = GetActorRotation();
		FVector  SpawnLocation = GetActorLocation();

		GetWorld()->SpawnActor<AItem>(FireBall, GetActorLocation(), GetActorRotation());
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

void AMain::WaveSkillActivation() {

	FVector ActorLoc = GetActorLocation();
	FVector ActorForward = GetActorForwardVector();
	FVector StartLoc = ActorLoc; // ������ ���� ����.
	FVector EndLoc = ActorLoc + (ActorForward * 1000.0f); // ������ ������ ����.

	float Radius = 100.f;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes; // ��Ʈ ������ ������Ʈ ������.
	TEnumAsByte<EObjectTypeQuery> Pawn = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn);
	ObjectTypes.Add(Pawn);

	TArray<AActor*> IgnoreActors; // ������ ���͵�.

	TArray<FHitResult> HitResults; // ��Ʈ ��� �� ���� ����.

	bool Result = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		StartLoc,
		EndLoc,
		Radius,
		ObjectTypes,
		false,
		IgnoreActors, // ������ ���� ���ٰ��ص� null�� ���� �� ����.
		EDrawDebugTrace::ForDuration,	// �����
		HitResults,
		true
		// ���� �ؿ� 3���� �⺻ ������ ������. �ٲٷ��� ������ ��.
		//, FLinearColor::Red
		//, FLinearColor::Green
		//, 5.0f
		);

	if (Result == true)
	{
		// FVector ImpactPoint = HitResult.ImpactPoint;
		// HitResult���� �ʿ��� ������ ����ϸ� ��.
		for (auto HitResult : HitResults) {
			AEnemy* Enemy = Cast<AEnemy>(HitResult.Actor);
			if (Enemy) 
			{
				//@@@@@ TODO : ���� ���� �� ��ų�� ��ƼŬ�� ���嵵 ������ ������
				if (Enemy->HitParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, HitResult.ImpactPoint, FRotator(0.f), true);
				}
				if (Enemy->HitSound)
				{
					UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
				}
				if (DamageTypeClass)
				{
					// TakeDamage �Լ��� �����Ǿ� �������� ����
					UGameplayStatics::ApplyDamage(Enemy, WaveSkillDamage, MainInstigator, this, DamageTypeClass);	// ���ش��, ���ط�, ��Ʈ�ѷ�(������), ���� ������, �ջ�����
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
		PlaySkillMontage("Skill02End");
	}
}

