// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowPickUp.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Main.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AArrowPickUp::AArrowPickUp()
{
	CollisionVolume->SetSphereRadius(100.f);
	InitialLifeSpan = 60.f;

	AmmoToAdd = 1;
}

void AArrowPickUp::BeginPlay()
{
	Super::BeginPlay();

	CollisionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArrowPickUp::OnOverlapBegin);
	CollisionVolume->OnComponentEndOverlap.AddDynamic(this, &AArrowPickUp::OnOverlapEnd);

	FTimerHandle SetGravityHandle;
	GetWorld()->GetTimerManager().SetTimer(SetGravityHandle, FTimerDelegate::CreateLambda([&]()
		{
			SetActorEnableCollision(true);
		}), 0.5f, false);
}

void AArrowPickUp::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	AMain* Main = Cast<AMain>(OtherActor);
	if (nullptr == Main) return;
	
	if (Main->Arrows_ < Main->MaxArrows_)
	{
		if(PickUpSound)	UGameplayStatics::PlaySoundAtLocation(this, PickUpSound, Mesh->GetComponentLocation());
		
		Main->AddArrows(AmmoToAdd);

		Destroy();
	}
}

void AArrowPickUp::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}