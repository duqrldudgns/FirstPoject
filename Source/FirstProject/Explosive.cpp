// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Main.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"

AExplosive::AExplosive()
{
	Damage = 15.f;

	ExplosiveInstigator = nullptr;
}

void AExplosive::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("Explosive::Overlap Begin."));

	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);	
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Main || Enemy)	
		{
			Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
			
			FHitResult HitResult(ForceInit);
			HitResult.Location = GetActorLocation();
			UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetActorLocation(), HitResult, ExplosiveInstigator, this, DamageTypeClass);

			Destroy();
		}
	}
}

void AExplosive::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	//UE_LOG(LogTemp, Warning, TEXT("Explosive::Overlap End."));
}
