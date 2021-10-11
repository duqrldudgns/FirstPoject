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

	MaxHealth = 1000.f;
	Health = 1000.f;

	Damage = 30.f;

	GetCharacterMovement()->MaxWalkSpeed = 600.f;

	AIControllerClass = ABossSevarogAIController::StaticClass();
	
	DetectRadius = 4000.f;

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
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);

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
	GetCharacterMovement()->MaxWalkSpeed = 2000.f;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);	// Player에 대한 충돌 설정

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
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
}

void ABossSevarog::CapsuleOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("TEST"));
	AMain* Main = Cast<AMain>(OtherActor);
	if (Main)
	{
		FHitResult hitResult(ForceInit);

		if (DamageTypeClass)
		{
			UGameplayStatics::ApplyPointDamage(Main, Damage, GetActorLocation(), hitResult, Controller, this, DamageTypeClass); // 포인트 데미지
		}
	}
}

void ABossSevarog::ThunderstrokeAttack()
{
	if (!bAttacking && Alive())
	{
		bAttacking = true;
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);

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
				}
			}
		}
		if (AnimInstance && ThunderstrokeMontage)
		{
			AnimInstance->Montage_Play(ThunderstrokeMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Thunderstroke"), ThunderstrokeMontage);
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
	}
}