// Fill out your copyright notice in the Description page of Project Settings.


#include "BossSevarog.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "BossSevarogAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Main.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Thunderstroke.h"
#include "PositionDecal.h"
#include "Sound/SoundCue.h"
#include "SpawnVolume.h"

ABossSevarog::ABossSevarog()
{
	AuraParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("AuraParticle"));
	AuraParticle->SetupAttachment(GetRootComponent());
	AuraParticle->bAutoActivate = true;
	AuraParticle->SetRelativeScale3D(FVector(10.0f));

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("ParticleSystem'/Game/ParagonSevarog/FX/Particles/P_Sevarog_Homescreen_Hammer.P_Sevarog_Homescreen_Hammer'"));
	if (ParticleAsset.Succeeded())
	{
		AuraParticle->SetTemplate(ParticleAsset.Object);
	}
	GetCapsuleComponent()->SetCapsuleSize(130.f, 260.f);

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackMontage(TEXT("AnimMontage'/Game/Enemies/Sevarog/Animations/SevarogRush_MT.SevarogRush_MT'"));
	if (AttackMontage.Succeeded())
	{
		RushAttackMontage = AttackMontage.Object;
	}

	MaxHealth = 2000.f;
	Health = 2000.f;

	Damage = 30.f;

	GetCharacterMovement()->MaxWalkSpeed = 600.f;

	AIControllerClass = ABossSevarogAIController::StaticClass();
	
	DetectRadius = 4000.f;

	AttDelay = 8.f;

}

void ABossSevarog::BeginPlay()
{
	Super::BeginPlay();
	
	Controller = Cast<ABossSevarogAIController>(GetController());

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABossSevarog::CapsuleOnOverlapBegin);

	
}

void ABossSevarog::RushAttack()
{
	if (!bAttacking && Alive())
	{
		bAttacking = true;
		bCanAttack = false;
		bDamagedIng = false;

		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);

		AttackDelay();

		if (Controller)
		{
			AMain* Main = Cast<AMain>(Controller->GetBlackboardComponent()->GetValueAsObject(ABossSevarogAIController::TargetKey));
			if (Main) {
				FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Main->GetActorLocation());	// 목표 방향 Rotation을 반환
				FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);

				SetActorRotation(LookAtRotationYaw);
			}
		}
		if (AnimInstance && RushAttackMontage)
		{
			AnimInstance->Montage_Play(RushAttackMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Rush"), RushAttackMontage);
		}
	}
}

void ABossSevarog::Rush()
{
	GetCharacterMovement()->MaxWalkSpeed = 8000.f;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);	// Player에 대한 충돌 설정
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);

	if (Controller)
	{
		AMain* Main = Cast<AMain>(Controller->GetBlackboardComponent()->GetValueAsObject(ABossSevarogAIController::TargetKey));
		if (Main)
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s"), *Main->GetActorLocation().ToString());
			FVector TargetLoc = Main->GetFloor() + GetActorForwardVector() * 1000.f;;

			AIController->MoveToLocation(TargetLoc);
		}
	}
}

void ABossSevarog::AttackEnd() 
{
	Super::AttackEnd();

	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);

}

void ABossSevarog::CapsuleOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("TEST"));
	AMain* Main = Cast<AMain>(OtherActor);
	if (Main)
	{
		FHitResult hitResult(ForceInit);

		if (DamageTypeClass)
		{
			UGameplayStatics::ApplyPointDamage(Main, Damage, GetActorLocation(), hitResult, Controller, this, Main->Rush); // 포인트 데미지
		}
	}
}

void ABossSevarog::ThunderstrokeAttack()
{
	if (!bAttacking && Alive())
	{
		bAttacking = true;
		bCanAttack = false;
		bDamagedIng = false;

		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);

		AttackDelay();

		if (AnimInstance && ThunderstrokeMontage)
		{
			AnimInstance->Montage_Play(ThunderstrokeMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Thunderstroke"), ThunderstrokeMontage);
		}

		CreateThunderDecal();
	}
}

void ABossSevarog::CreateThunderDecal()
{
	if (Controller)
	{
		AMain* Main = Cast<AMain>(Controller->GetBlackboardComponent()->GetValueAsObject(ABossSevarogAIController::TargetKey));
		if (Main) {
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Main->GetActorLocation());	// 목표 방향 Rotation을 반환
			FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);

			SetActorRotation(LookAtRotationYaw);

			if (PositionDecal)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;

				ThunderSpawnLoc = Main->GetFloor();

				GetWorld()->SpawnActor<APositionDecal>(PositionDecal, ThunderSpawnLoc, FRotator(0.f), SpawnParams);

				int32 ThunderSpawnCount = 30;
				for (int32 i = 0; i < ThunderSpawnCount; i++)
				{
					FVector Point = GetRandomPoint();
					Point.Z = ThunderSpawnLoc.Z;
					RandomPoints.Push(Point);

					GetWorld()->SpawnActor<APositionDecal>(PositionDecal, Point, FRotator(0.f), SpawnParams);
				}
			}
		}
	}
}

void ABossSevarog::ActiveThunderstroke()
{
	if (Thunderstroke)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		AThunderstroke* Thunder = GetWorld()->SpawnActor<AThunderstroke>(Thunderstroke, ThunderSpawnLoc, FRotator(0.f), SpawnParams);
		Thunder->SetInstigator(GetController());

		for (auto Point : RandomPoints)
		{
			AThunderstroke* Thunders = GetWorld()->SpawnActor<AThunderstroke>(Thunderstroke, Point, FRotator(0.f), SpawnParams);
			Thunders->SetInstigator(GetController());
		}
		RandomPoints.Empty();
	}
}

FVector ABossSevarog::GetRandomPoint()
{
	FVector Origin = GetActorLocation();
	FVector Extent = FVector(3000.f, 3000.f, 0.f);

	FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);	// Origin을 기준으로 Extent만큼 그 크기 내에서 랜덤한 포인트를 설정

	return Point;
}

float ABossSevarog::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (DecrementHealth(DamageAmount) && !bAttacking)
	{
		AController* Coontroller = Cast<AController>(EventInstigator);
		if (Coontroller)	//누가 때리던 간에 그냥 맞아야함, 컨트롤러 연결하는 연습이라도 할겸 연결해봐야함
		{
			AMain* Main = Cast<AMain>(Coontroller->GetPawn());
			if (Main && AnimInstance && DamagedMontage && !bDamagedIng)
			{
				FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Main->GetActorLocation());
				LookAtRotation.Pitch = 0.f;
				LookAtRotation.Roll = 0.f;		// (0.f, LookAtRotation.Yaw, 0.f);
				//UE_LOG(LogTemp, Warning, TEXT("%s"), *LookAtRotation.ToString());

				FRotator HitRotation = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, GetActorRotation());
				UE_LOG(LogTemp, Warning, TEXT("%s"), *HitRotation.ToString());

				UE_LOG(LogTemp, Warning, TEXT("%d"), bAttacking);
				if (DamageEvent.DamageTypeClass == Main->Basic)
				{
					//bDamagedIng = true;

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
				else
				{
					bDamagedIng = true;
					bAttacking = false;

					AnimInstance->Montage_Play(DamagedMontage, 1.0f);
					AnimInstance->Montage_JumpToSection(FName("KnockDown"), DamagedMontage);
				}
			}
		}
	}

	AController* Coontroller = Cast<AController>(EventInstigator);
	if (Coontroller)	//누가 때리던 간에 그냥 맞아야함, 컨트롤러 연결하는 연습이라도 할겸 연결해봐야함
	{
		AMain* Main = Cast<AMain>(Coontroller->GetPawn());
		if (Main && AnimInstance && DamagedMontage && !bDamagedIng)
		{
			// Show DamageNumbers
			if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))	// PointDamage 받기
			{
				const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
				float DamageNumbers = PointDamageEvent->Damage;
				bool Headshot = false;
				if (0 == (PointDamageEvent->HitInfo.BoneName).Compare(FName(TEXT("Head"))))
				{
					DamageNumbers *= 1.5f;
					Headshot = true;
				}
				Main->SpawnDamageNumbers(PointDamageEvent->HitInfo.Location, Headshot, DamageNumbers);

				//헤드샷일경우 추가 데미지
				if (!DecrementHealth(DamageNumbers - PointDamageEvent->Damage)) return DamageNumbers;
			}
		}
	}

	return DamageAmount;
}

void ABossSevarog::ShowSmashRange()
{
	if (PositionDecal)
	{
		SmashLoc = GetActorLocation() + FVector(0.f, 0.f, -250.f) + GetActorForwardVector() * 400.f + GetActorRightVector() * 150.f;
		FTransform Transform;
		Transform.SetLocation(SmashLoc);
		Transform.SetScale3D(FVector(14.f));
		GetWorld()->SpawnActor<APositionDecal>(PositionDecal, Transform);
	}
}

void ABossSevarog::Smash()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	TEnumAsByte<EObjectTypeQuery> Target = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	ObjectTypes.Add(Target);	//Player

	TArray<AActor*> IgnoreActors;
	TArray<FHitResult> HitResults;

	bool Result = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		SmashLoc,
		SmashLoc,
		3000.f,
		ObjectTypes,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,	// 디버그
		HitResults,
		true
		// 여기 밑에 3개는 기본 값으로 제공됨. 바꾸려면 적으면 됨.
		//, FLinearColor::Red
		//, FLinearColor::Green
		//, 5.0f
	);

	if (Result)
	{
		for (auto HitResult : HitResults)
		{
			AMain* Main = Cast<AMain>(HitResult.GetActor());
			if (Main)
			{
				if (Main->GetCharacterMovement()->IsWalking())
				{ 
					UGameplayStatics::ApplyPointDamage(Main, 50.f, SmashLoc, HitResult, Controller, this, Main->Rush);
					FVector LaunchVelocity = Main->GetActorForwardVector() * -5000.f + Main->GetActorUpVector() * 100.f;
					Main->LaunchCharacter(LaunchVelocity, false, false);
				}
			}
		}
	}

	if (SmashSound) UGameplayStatics::PlaySoundAtLocation(this, SmashSound, SmashLoc);
	
	if (SmashParticle) UGameplayStatics::SpawnEmitterAtLocation(this, SmashParticle, SmashLoc, FRotator::ZeroRotator, FVector(11.f, 11.f, 6.f));
}

void ABossSevarog::CreateEnemySpawner()
{
	if (EnemySpawner)
	{
		FVector SpawnLoc = GetActorLocation() + FVector(0.f, 0.f, -200.f) + GetActorForwardVector() * 400.f;
		FTransform Transform;
		Transform.SetLocation(SpawnLoc);
		Transform.SetScale3D(FVector(12.f, 12.f, 1.f));
		GetWorld()->SpawnActor<ASpawnVolume>(EnemySpawner, Transform);
	}
}

void ABossSevarog::SpawnEnemyAttack()
{
	if (!bAttacking && Alive())
	{
		bAttacking = true;
		bCanAttack = false;
		bDamagedIng = false;

		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);

		AttackDelay();

		if (AnimInstance && SpawnEnemyMontage)
		{
			AnimInstance->Montage_Play(SpawnEnemyMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("SpawnEnemy"), SpawnEnemyMontage);
		}
	}
}