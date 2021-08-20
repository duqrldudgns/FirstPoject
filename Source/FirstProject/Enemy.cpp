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
	CombatCollision->SetupAttachment(GetMesh(), FName("WeaponSocket"));	// 해당 이름을 가진 소켓에 콜리젼박스를 붙임

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

	// 충돌 유형 지정
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // QueryOnly : 물리학 계산 하지 않음
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);	// 모든Dynamic 요소에 대한 충돌
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	// 그 충돌에 대한 반응은 무시하고
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);	// Pawn에 대한 충돌만 Overlap으로 설정

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);	// pickup을 agro로 터트리지 않기 위함

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
			//GetWorldTimerManager().ClearTimer(AttackTimer);		//@@@@@FIX TODO : 이걸 끄면  공격이 끝난후 타이머에 들어갔을 때 캐릭터가 공격범위를 벗어나면 제자리에서 한번더 공격함

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
				// 스켈레톤에 들어가 설정해둔 소켓 이름인 "TipSocket"을 가져와 그 위치에 효과 발생		-	Weapon 설정과 비슷하지만 다르니 참고
				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
				if (TipSocket)
				{
					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}

			if (Main->HitSound)	// 때렸을 때 Enemy에서 나는 소리
			{
				UGameplayStatics::PlaySound2D(this, Main->HitSound);
			}
			if (DamageTypeClass)
			{
				// TakeDamage 함수와 연동되어 데미지를 입힘
				UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);	// 피해대상, 피해량, 컨트롤러(가해자), 피해 유발자, 손상유형
			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AEnemy::MoveToTarget(class AMain* Target)	//@@@@@FIX ERROR : 네비게이션박스 밖에 있는 캐릭터 발견 시 에러남
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);
	CombatTarget = nullptr;

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);		// 이동 목표 : 배우
		MoveRequest.SetAcceptanceRadius(5.0f);	// 목표 Actor에 도착했을 때 목표 Actor와의 거리

		FNavPathSharedPtr NavPath;				// 목표를 따라가기 위한 경로 설정?

		AIController->MoveTo(MoveRequest, &NavPath);	//이동


		// Debug
		auto PathPoints = NavPath->GetPathPoints();		//경로를 정한 것을 배열로 저장
		for (auto Point : PathPoints)
		{
			FVector Location = Point.Location;
			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 16, FLinearColor::Red, 5.f, 1.5f);
		}
	}
}

void AEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // QueryOnly : 물리학 계산 하지 않음
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

//@@@@@FIX ERROR : 공격할 때 점프하고있으면 에러남
//@@@@@FIX TODO : CombatSphereOnOverlapBegin을 벗어나지않게 Enemy의 뒤로 돌면 앞만 계속 때리고 있음 - Rotator주면될듯
void AEnemy::AttackEnd()
{
	bAttacking = false;
	if (bOverlappingCombatSphere)
	{
																							//@@@@@FIX TODO : 어택에 타이머를 두는게 아니라 공격이 끝나면 2초간 쉬었다가 공격이든 움직이든 하고싶음
		//GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackDelay); //@@@@@FIX TODO : 공격이 끝난후 타이머에 들어갔을 때 캐릭터가 공격범위를 벗어나면 멈춰있음
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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();		//애니메이션 인스턴스를 가져옴
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
	GetMesh()->bPauseAnims = true;			// 애니메이션 정지
	GetMesh()->bNoSkeletonUpdate = true;	// 스켈레톤 업데이트 정지

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive()	// 살아있으면 true
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::Disappear()
{
	Destroy();
}