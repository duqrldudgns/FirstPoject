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
	bAttacking = false;
	bInterpToEnemy = false;
	bHasCombatTarget = false;
	bMovingForward = false;
	bMovingRight = false;
	
	//Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 20.f;
	MinSprintStamina = 30.f; 

	InterpSpeed = 15.f;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());
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

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);	// ��ǥ ���� Rotation�� ��ȯ
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
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

	PlayerInputComponent->BindAction("Interaction", IE_Pressed, this, &AMain::InteractionKeyDown);
	PlayerInputComponent->BindAction("Interaction", IE_Released, this, &AMain::InteractionKeyUp);
	
	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);
	
	// Tick ó�� ��� ���� �� .. ����??
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);	// �÷��̾���Ʈ�ѷ��� yaw���� ����
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);
	
}


void AMain::MoveForward(float Value)
{
	bMovingForward = false;		// �̷��� �ص� ������ �� true�� ���� �Ǵ� ��
	if (Controller != nullptr && Value != 0.0f && (!bAttacking) && Alive())	// ���ݽ� ������ ����
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
	if (Controller != nullptr && Value != 0.0f && (!bAttacking) && Alive())	// ���ݽ� ������ ����
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
	if (Alive())
	{
		ACharacter::Jump();	// =  Super::Jump();
	}
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

void AMain::DecrementHealth(float Amount)
{
	Health -= Amount;

	if (Health <= 0.f)
	{
		Die();
	}
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
	if (!Alive()) return;

	SetMovementStatus(EMovementStatus::EMS_Dead);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//�ִϸ��̼� �ν��Ͻ��� ������
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
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

	if (EquippedWeapon && Alive())
	{
		Attack();
	}
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::InteractionKeyDown()
{
	bInteractionKeyDown = true;
	if (ActiveOverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
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
	if (!bAttacking)	// ���� ���� �����ϴ� ���� ����
	{
		bAttacking = true;
		if(CombatTarget) SetInterpToEnemy(true);	// ���� ���� ���� ������� ��Ҵٸ� ���� ������ ������ ���·� ����

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//�ִϸ��̼� �ν��Ͻ��� ������
		if (AnimInstance && CombatMontage)
		{
			int32 Section = FMath::RandRange(0, 1);
			switch (Section)	// ���� ������ ���� ����
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 1.0f);	// �ش� ��Ÿ�ָ� �ش� �ӵ��� ����
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);	//�ش� ���� ����
				break;

			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.5f);
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				break;

			default:
				;
			}
		}
	}
}

void AMain::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);

	if (bLMBDown)	// ������ �����µ��� ���콺�� �����ִٸ� ��� ����
	{
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
