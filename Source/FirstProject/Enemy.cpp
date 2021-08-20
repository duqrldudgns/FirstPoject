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

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(80.f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("WeaponSocket"));	// �ش� �̸��� ���� ���Ͽ� �ݸ����ڽ��� ����

	bOverlappingCombatSphere = false;
	bAttacking = false;
	bHasValidTarget = false;

	Health = 75.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackDelay = 2.f;
	DeathDelay = 3.f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	AIController = Cast<AAIController>(GetController());

	// �浹 ���� ����
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // QueryOnly : ������ ��� ���� ����
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);	// ���Dynamic ��ҿ� ���� �浹
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	// �� �浹�� ���� ������ �����ϰ�
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);	// Pawn�� ���� �浹�� Overlap���� ����

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);	// pickup�� agro�� ��Ʈ���� �ʱ� ����

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

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			MoveToTarget(Main);
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);

			if (AIController)
			{
				AIController->StopMovement();
			}
		
			Main->UpdateCombatTarget();
			
			CombatTarget = nullptr;
			bHasValidTarget = false;
		}
	}
}

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
			Attack();
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
			//GetWorldTimerManager().ClearTimer(AttackTimer);		//@@@@@FIX TODO : �̰� ����  ������ ������ Ÿ�̸ӿ� ���� �� ĳ���Ͱ� ���ݹ����� ����� ���ڸ����� �ѹ��� ������

			if (Main->CombatTarget == this)
			{
				Main->UpdateCombatTarget();
			}
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
			if (Main->HitParticles)
			{
				// ���̷��濡 �� �����ص� ���� �̸��� "TipSocket"�� ������ �� ��ġ�� ȿ�� �߻�		-	Weapon ������ ��������� �ٸ��� ����
				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
				if (TipSocket)
				{
					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}

			if (Main->HitSound)	// ������ �� Enemy���� ���� �Ҹ�
			{
				UGameplayStatics::PlaySound2D(this, Main->HitSound);
			}
			if (DamageTypeClass)
			{
				// TakeDamage �Լ��� �����Ǿ� �������� ����
				UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);	// ���ش��, ���ط�, ��Ʈ�ѷ�(������), ���� ������, �ջ�����
			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AEnemy::MoveToTarget(class AMain* Target)	//@@@@@FIX ERROR : �׺���̼ǹڽ� �ۿ� �ִ� ĳ���� �߰� �� ������
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);
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
	if (!bAttacking && Alive() && bHasValidTarget)
	{
		bAttacking = true;
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);

		if (AIController)
		{
			AIController->StopMovement();
		}

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
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
	if (bOverlappingCombatSphere)
	{
																							//@@@@@FIX TODO : ���ÿ� Ÿ�̸Ӹ� �δ°� �ƴ϶� ������ ������ 2�ʰ� �����ٰ� �����̵� �����̵� �ϰ����
		//GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackDelay); //@@@@@FIX TODO : ������ ������ Ÿ�̸ӿ� ���� �� ĳ���Ͱ� ���ݹ����� ����� ��������
		Attack();
	}
}

void AEnemy::PlaySwingSound()
{
	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

void AEnemy::DecrementHealth(float Amount, AActor* Causer)
{
	Health -= Amount;

	if (Health <= 0.f)
	{
		Die(Causer);
	}
}

void AEnemy::Die(AActor* Causer)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	
	AMain* Main = Cast<AMain>(CombatTarget);
	if (Main)
	{
		Main->UpdateCombatTarget();
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//�ִϸ��̼� �ν��Ͻ��� ������
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	

}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	DecrementHealth(DamageAmount, DamageCauser);

	return DamageAmount;
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