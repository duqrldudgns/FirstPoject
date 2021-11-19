// Fill out your copyright notice in the Description page of Project Settings.


#include "Arrow.h"
#include "Bow.h"
#include "Main.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AArrow::AArrow()
{
	CollisionVolume->SetCollisionProfileName("BlockAll");
	CollisionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
	
	DelayBeforeGravity = 0.1f;
	RadiusCheck = 4.f;
}

void AArrow::OnConstruction(const FTransform& Transform)
{
	ProjectileMovementComponent->InitialSpeed = ProjectileSpeed;
}

void AArrow::BeginPlay()
{
	Super::BeginPlay();

	//발사체의 회전이 이동 방향에 맞춰 매 프레임 업데이트
	ProjectileMovementComponent->bRotationFollowsVelocity = true;	

	//A little delay makes it look a bit better.
	//Set Delay(0.05)
	IdleParticlesComponent->SetActive(true);

	//Games like zelda havea little delay before enabling gravity. 
	FTimerHandle SetGravityHandle;
	GetWorld()->GetTimerManager().SetTimer(SetGravityHandle, FTimerDelegate::CreateLambda([&]()
		{
			ProjectileMovementComponent->ProjectileGravityScale = GravityScale;
		}), DelayBeforeGravity, false);

}

// Called every frame
void AArrow::Tick(float DeltaTime)
{
	RotationUpdate(GetActorRotation());
}

void AArrow::RotationUpdate(FRotator NewRotation)
{
	SetActorRotation(NewRotation);
}

void AArrow::OnOtherHit(AActor* Other, FVector HitLocation)
{
	if (nullptr == BowReference) return;

	//in the rare occassion that it hits the player character, it's destroyed.
	AMain* Main = Cast<AMain>(Other);
	if (Main == BowReference->CharReference) Destroy();
	
	// Added this to get the appropriate hit info. 
	FVector StartLoc = CheckImpactLocation->GetComponentLocation();
	FVector EndLoc = CheckImpactLocation->GetComponentLocation() + FVector(0.f, 0.f, 5.f);

	TArray<AActor*> IgnoreActors;
	FHitResult HitResult;

	bool Result = false;
	while (!Result)
	{
		Result = UKismetSystemLibrary::SphereTraceSingle(
			GetWorld(),
			StartLoc,
			EndLoc,
			RadiusCheck,
			ETraceTypeQuery::TraceTypeQuery4,	//PlayerProjectile
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
		RadiusCheck += 4.f;
	}

	BowReference->DamageEvent(HitResult, Damage, DamageTypeClass);

	Destroy();
}