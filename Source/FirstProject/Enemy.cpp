// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Main.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"
#include "Weapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.h"
#include "Components/DecalComponent.h"
#include "Components/WidgetComponent.h"


// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	//AgroSphere->SetupAttachment(GetRootComponent());
	//AgroSphere->InitSphereRadius(600.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(80.f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("WeaponSocket"));	// �ش� �̸��� ���� ���Ͽ� �ݸ����ڽ��� ����

	SelectDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("SelectDecal"));
	SelectDecal->SetupAttachment(GetRootComponent());
	SelectDecal->SetVisibility(false);
	SelectDecal->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	static ConstructorHelpers::FObjectFinder<UMaterial> Decal(TEXT("Material'/Game/GameplayMechanics/test/CharaSelectDecal/CharSelectDecal_MT.CharSelectDecal_MT'"));
	if (Decal.Succeeded())
	{
		SelectDecal->SetDecalMaterial(Decal.Object);
	}

	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(GetRootComponent());
	HealthBar->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	HealthBar->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBar->SetDrawSize(FVector2D(120.f, 15.f));
	HealthBar->SetVisibility(false);

	bOverlappingCombatSphere = false;
	bAttacking = false;
	bHasValidTarget = false;
	bDamagedIng = false;
	bHitOnce = false;

	Health = 75.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackDelay = 2.f;
	DeathDelay = 5.0f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	DamageTypeClass = UDamageType::StaticClass();

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;	//������ ��ġ�ϰų� ���� �����Ǵ� Enemy�� EnemyAIController�� ���踦 �ްԵ�

	DetectRadius = 600.f;

	DisplayHealthBarTime = 5.f;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = GetMesh()->GetAnimInstance();		//�ִϸ��̼� �ν��Ͻ��� ������

	AIController = Cast<AAIController>(GetController());

	// �浹 ���� ����
	GetCapsuleComponent()->SetCollisionProfileName("Enemy");
	//GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);	//Enemy
	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Overlap);
	//GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);

	// ���� �浹 ���� ����
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // QueryOnly : ������ ��� ���� ����
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);	// ���Dynamic ��ҿ� ���� �浹
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	// �� �浹�� ���� ������ �����ϰ�
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);	// Pawn�� ���� �浹�� Overlap���� ����

	//AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	//AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);
	//AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);	// pickup�� agro�� ��Ʈ���� �ʱ� ����

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
//{
//	if (OtherActor && Alive())
//	{
//		AMain* Main = Cast<AMain>(OtherActor);
//		if (Main)
//		{
//			MoveToTarget(Main);
//		}
//	}
//}
//
//void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	if (OtherActor)
//	{
//		AMain* Main = Cast<AMain>(OtherActor);
//		if (Main)
//		{
//			if (Alive()) 
//			{
//				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
//			}
//			GetCharacterMovement()->MaxWalkSpeed = 300.f;
//
//			if (AIController)
//			{
//				AIController->StopMovement();
//			}
//		
//			Main->UpdateCombatTarget();
//			
//			CombatTarget = nullptr;
//			bHasValidTarget = false;
//		}
//	}
//}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			Main->UpdateCombatTarget();

			CombatTarget = Main;
			bHasValidTarget = true;

			bOverlappingCombatSphere = true;

			//if (GetWorldTimerManager().IsTimerActive(TimerHandle))
			//{
			//	TimerRemaining = GetWorldTimerManager().GetTimerRemaining(TimerHandle);
			//	GetWorldTimerManager().ClearTimer(TimerHandle);

			//	GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemy::Attack, TimerRemaining);
			//}
			//else if (TimerRemaining > 0)
			//{
			//	GetWorldTimerManager().ClearTimer(TimerHandle);

			//	GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemy::Attack, TimerRemaining);
			//}
			//else 
			//{
			//	Attack();
			//}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			bOverlappingCombatSphere = false;

			if (Main->CombatTarget == this)
			{
				Main->UpdateCombatTarget();
			}
			//if (GetWorldTimerManager().IsTimerActive(TimerHandle) && !bAttacking)
			//{
			//	TimerRemaining = GetWorldTimerManager().GetTimerRemaining(TimerHandle);
			//	GetWorldTimerManager().PauseTimer(TimerHandle);

			//	MoveToTarget(Main);
			//}
		}
	}
}

void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			//if (Main->HitParticles)
			//{
			//	// ���̷��濡 �� �����ص� ���� �̸��� "TipSocket"�� ������ �� ��ġ�� ȿ�� �߻�		-	Weapon ������ ��������� �ٸ��� ����
			//	const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
			//	if (TipSocket)
			//	{
			//		FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
			//		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), false);
			//	}
			//}

			FHitResult hitResult(ForceInit);

			if (DamageTypeClass)
			{
				//UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);	// ���ش��, ���ط�, ��Ʈ�ѷ�(������), ���� ������, �ջ�����
				UGameplayStatics::ApplyPointDamage(Main, Damage, GetActorLocation(), hitResult, AIController, this, DamageTypeClass); // ����Ʈ ������

			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AEnemy::MoveToTarget(class AMain* Target)	//@@@@@FIX ERROR : �׺���̼ǹڽ� �ۿ� �ִ� ĳ���� �߰� �� ������
{
	if (Alive())
	{
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);
	}
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

	CombatTarget = nullptr;

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);		// �̵� ��ǥ : ���
		MoveRequest.SetAcceptanceRadius(5.0f);	// ��ǥ Actor�� �������� �� ��ǥ Actor���� �Ÿ�

		FNavPathSharedPtr NavPath;				// ��ǥ�� ���󰡱� ���� ��� ����?

		AIController->MoveTo(MoveRequest, &NavPath);	//�̵�


		// Debug
		auto PathPoints = NavPath->GetPathPoints();		//��θ� ���� ���� �迭�� ����
		for (auto Point : PathPoints)
		{
			FVector Location = Point.Location;
			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 16, FLinearColor::Red, 5.f, 1.5f);
		}
	}
}

void AEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // QueryOnly : ������ ��� ���� ����
}


void AEnemy::DeActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::Attack()
{
	if (!bAttacking && Alive())// && bHasValidTarget)
	{
		bAttacking = true;
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);

		if (AIController)
		{
			AIController->StopMovement();
		}

		if (AnimInstance && CombatMontage)
		{
			AnimInstance->Montage_Play(CombatMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
		}
	}
}

//@@@@@FIX ERROR : ������ �� �����ϰ������� ������
//@@@@@FIX TODO : CombatSphereOnOverlapBegin�� ������ʰ� Enemy�� �ڷ� ���� �ո� ��� ������ ���� - Rotator�ָ�ɵ�
void AEnemy::AttackEnd()
{
	bAttacking = false;

	OnAttackEnd.Broadcast();	// ��������Ʈ�� ȣ��

	//GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]()
	//{
	//	if (bOverlappingCombatSphere) {
	//		Attack();
	//	}
	//	else {
	//		if (CombatTarget)
	//		{
	//			MoveToTarget(CombatTarget);
	//		}
	//	}

	//	// TimerHandle �ʱ�ȭ
	//	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	//}), AttackDelay, false);	// �ݺ��Ϸ��� false�� true�� ����
}

void AEnemy::PlaySwingSound()
{
	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

bool AEnemy::DecrementHealth(float Amount)
{
	Health -= Amount;

	if (Health <= 0.f)
	{
		Die();
		return false;
	}

	return true;
}

void AEnemy::Die()
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	GetCharacterMovement()->MaxWalkSpeed = 0.f;

	AMain* Main = Cast<AMain>(CombatTarget);
	if (Main)
	{
		Main->UpdateCombatTarget();
	}

	//if (GetWorldTimerManager().IsTimerActive(TimerHandle))
	//{
	//	GetWorldTimerManager().ClearTimer(TimerHandle);
	//}

	if (AnimInstance && DamagedMontage)
	{
		AnimInstance->Montage_Play(DamagedMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"), DamagedMontage);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (DecrementHealth(DamageAmount))
	{
		AController* Coontroller = Cast<AController>(EventInstigator);
		if (Coontroller)	//���� ������ ���� �׳� �¾ƾ���, ��Ʈ�ѷ� �����ϴ� �����̶� �Ұ� �����غ�����
		{
			AMain* Main = Cast<AMain>(Coontroller->GetPawn());
			if (Main && AnimInstance && DamagedMontage)
			{
				FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Main->GetActorLocation());
				LookAtRotation.Pitch = 0.f;
				LookAtRotation.Roll = 0.f;		// (0.f, LookAtRotation.Yaw, 0.f);
				//UE_LOG(LogTemp, Warning, TEXT("%s"), *LookAtRotation.ToString());

				FRotator HitRotation = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, GetActorRotation());
				UE_LOG(LogTemp, Warning, TEXT("%s"), *HitRotation.ToString());

				if (DamageEvent.DamageTypeClass == Main->Basic && !bAttacking)
				{
					bDamagedIng = true;

					if (GetCharacterMovement()->IsWalking())
					{
						if (-45.f <= HitRotation.Yaw && HitRotation.Yaw < 45.f)
						{
							AnimInstance->Montage_Play(DamagedMontage, 1.0f);
							AnimInstance->Montage_JumpToSection(FName("FrontHit"), DamagedMontage);
						}
						else if (-135.f <= HitRotation.Yaw && HitRotation.Yaw < -45.f)
						{
							AnimInstance->Montage_Play(DamagedMontage, 1.0f);
							AnimInstance->Montage_JumpToSection(FName("LeftHit"), DamagedMontage);
						}
						else if (45.f <= HitRotation.Yaw && HitRotation.Yaw < 135.f)
						{
							AnimInstance->Montage_Play(DamagedMontage, 1.0f);
							AnimInstance->Montage_JumpToSection(FName("RightHit"), DamagedMontage);
						}
						else
						{
							AnimInstance->Montage_Play(DamagedMontage, 1.0f);
							AnimInstance->Montage_JumpToSection(FName("BackHit"), DamagedMontage);
						}
					}
					else	// ���� ���� ���� ��
					{
						GetCharacterMovement()->SetMovementMode(MOVE_Flying);
						GetCharacterMovement()->Velocity = FVector(0.f);

						AnimInstance->Montage_Play(DamagedMontage, 1.0f);
						AnimInstance->Montage_JumpToSection(FName("UpperHit"), DamagedMontage);
					}
				}
				else if (DamageEvent.DamageTypeClass == Main->KnockDown)
				{
					bDamagedIng = true;
					AnimInstance->Montage_Play(DamagedMontage, 1.0f);
					AnimInstance->Montage_JumpToSection(FName("KnockDown"), DamagedMontage);
				}
				else if (DamageEvent.DamageTypeClass == Main->Upper)
				{
					bDamagedIng = true;
					AnimInstance->Montage_Play(DamagedMontage, 1.0f);
					AnimInstance->Montage_JumpToSection(FName("Upper"), DamagedMontage);

					SetActorRotation(LookAtRotation);

					FVector LaunchVelocity = GetActorUpVector() * 750.f;
					LaunchCharacter(LaunchVelocity, false, false);
				}
				else if (DamageEvent.DamageTypeClass == Main->Rush)
				{
					bDamagedIng = true;
					AnimInstance->Montage_Play(DamagedMontage, 1.0f);
					AnimInstance->Montage_JumpToSection(FName("Rush"), DamagedMontage);

					FRotator Rotation = Main->GetActorRotation();
					Rotation.Yaw -= 180.f;
					SetActorRotation(Rotation);

					FVector LaunchVelocity = GetActorForwardVector() * -1500.f + GetActorUpVector() * 40.f;
					LaunchCharacter(LaunchVelocity, false, false);
				}
			}
		}
	}

	return DamageAmount;
}

void AEnemy::AirHitEnd()
{
	if (GetCharacterMovement()->IsFlying())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
}

void AEnemy::DamagedDownEnd()
{
	if (GetCharacterMovement()->IsWalking())	// ���鿡 ��Ҵٸ�
	{
		if (AnimInstance && DamagedMontage)
		{
			AnimInstance->Montage_Play(DamagedMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("UpperEnd"), DamagedMontage);
		}
	}
}

void AEnemy::DamagedEnd()
{
	bDamagedIng = false;
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;			// �ִϸ��̼� ����
	GetMesh()->bNoSkeletonUpdate = true;	// ���̷��� ������Ʈ ����

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive()	// ��������� true
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::Disappear()
{
	Destroy();
}

void AEnemy::SetEnemyMovementStatus(EEnemyMovementStatus Status)
{
	EnemyMovementStatus = Status;
}

void AEnemy::PlayMontage(UAnimMontage* MontageName)
{
	AnimInstance->Montage_Play(MontageName, 1.0f);
}

void AEnemy::DisplayHealthBar()
{
	if (!HealthBar->GetVisibleFlag())
	{
		HealthBar->SetVisibility(true);
		GetWorldTimerManager().SetTimer(DisplayHealthBarHandle, this, &AEnemy::RemoveHealthBar, DisplayHealthBarTime);
	}
	else 
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		GetWorldTimerManager().SetTimer(DisplayHealthBarHandle, this, &AEnemy::RemoveHealthBar, DisplayHealthBarTime);
	}
}

void AEnemy::RemoveHealthBar()
{
	HealthBar->SetVisibility(false);
}

void AEnemy::ResetHitOnce()
{
	FTimerHandle WaitHandle;
	float WaitTime = 0.2f;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
		{
			bHitOnce = false;
		}), WaitTime, false);
}