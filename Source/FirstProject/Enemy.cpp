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
	CombatCollision->SetupAttachment(GetMesh(), FName("WeaponSocket"));	// 해당 이름을 가진 소켓에 콜리젼박스를 붙임

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
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;	//레벨에 배치하거나 새로 생성되는 Enemy는 EnemyAIController의 지배를 받게됨

	DetectRadius = 600.f;

	DisplayHealthBarTime = 5.f;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = GetMesh()->GetAnimInstance();		//애니메이션 인스턴스를 가져옴

	AIController = Cast<AAIController>(GetController());

	// 충돌 유형 지정
	GetCapsuleComponent()->SetCollisionProfileName("Enemy");
	//GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);	//Enemy
	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Overlap);
	//GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);

	// 무기 충돌 유형 지정
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // QueryOnly : 물리학 계산 하지 않음
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);	// 모든Dynamic 요소에 대한 충돌
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	// 그 충돌에 대한 반응은 무시하고
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);	// Pawn에 대한 충돌만 Overlap으로 설정

	//AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	//AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);
	//AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);	// pickup을 agro로 터트리지 않기 위함

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
			//	// 스켈레톤에 들어가 설정해둔 소켓 이름인 "TipSocket"을 가져와 그 위치에 효과 발생		-	Weapon 설정과 비슷하지만 다르니 참고
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
				//UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);	// 피해대상, 피해량, 컨트롤러(가해자), 피해 유발자, 손상유형
				UGameplayStatics::ApplyPointDamage(Main, Damage, GetActorLocation(), hitResult, AIController, this, DamageTypeClass); // 포인트 데미지

			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AEnemy::MoveToTarget(class AMain* Target)	//@@@@@FIX ERROR : 네비게이션박스 밖에 있는 캐릭터 발견 시 에러남
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

//@@@@@FIX ERROR : 공격할 때 점프하고있으면 에러남
//@@@@@FIX TODO : CombatSphereOnOverlapBegin을 벗어나지않게 Enemy의 뒤로 돌면 앞만 계속 때리고 있음 - Rotator주면될듯
void AEnemy::AttackEnd()
{
	bAttacking = false;

	OnAttackEnd.Broadcast();	// 델리게이트를 호출

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

	//	// TimerHandle 초기화
	//	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	//}), AttackDelay, false);	// 반복하려면 false를 true로 변경
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
		if (Coontroller)	//누가 때리던 간에 그냥 맞아야함, 컨트롤러 연결하는 연습이라도 할겸 연결해봐야함
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
					else	// 공중 공격 맞을 시
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
	if (GetCharacterMovement()->IsWalking())	// 지면에 닿았다면
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