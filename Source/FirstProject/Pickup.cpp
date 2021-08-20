// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Main.h"

APickup::APickup()
{
}

void APickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("Pickup::Overlap Begin."));
	
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)	//OtherActor와 Main이 같다면
		{
			OnPickupBP(Main);

			Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

			Main->PickupLocations.Add(GetActorLocation());

			Destroy();
		}
	}
}

void APickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	//UE_LOG(LogTemp, Warning, TEXT("Pickup::Overlap End."));
}
