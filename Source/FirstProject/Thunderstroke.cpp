 // Fill out your copyright notice in the Description page of Project Settings.


#include "Thunderstroke.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Enemy.h"
#include "Main.h"
#include "Sound/SoundCue.h"

AThunderstroke::AThunderstroke()
{
	CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
	CapsuleCollision->SetupAttachment(GetRootComponent());

	Damage = 80.f;

	InitialLifeSpan = 3.f;

	HitDelay = 0.5f;
}

void AThunderstroke::BeginPlay()
{
	Super::BeginPlay();

	CollisionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	CapsuleCollision->OnComponentBeginOverlap.AddDynamic(this, &AThunderstroke::CapsuleOverlapBegin);
	CapsuleCollision->OnComponentEndOverlap.AddDynamic(this, &AThunderstroke::CapsuleOverlapEnd);

	GetWorldTimerManager().SetTimer(HitTimer, this, &AThunderstroke::HitOverlap, HitDelay);

	if (OverlapSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, OverlapSound, GetActorLocation());
	}
}

void AThunderstroke::CapsuleOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("%d"), ExplosiveInstigator);

}

void AThunderstroke::CapsuleOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}


void AThunderstroke::HitOverlap()
{
	TArray<AActor*> Targets;
	TSubclassOf<AMain> TargetFilter;
	CapsuleCollision->GetOverlappingActors(Targets, TargetFilter);

	for (auto Target : Targets)
	{
		AMain* Main = Cast<AMain>(Target);
		if (Main)
		{
			UGameplayStatics::ApplyDamage(Main, Damage, ExplosiveInstigator, this, DamageTypeClass);	// 피해대상, 피해량, 컨트롤러(가해자), 피해 유발자, 손상유형
		}
	}

}