// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "Components/Boxcomponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Critter.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));

}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ASpawnVolume::GetSpawnPoint()
{
	FVector Extent = SpawningBox->GetScaledBoxExtent();
	FVector Origin = SpawningBox->GetComponentLocation();

	FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);	// Origin을 기준으로 Extent만큼 그 크기 내에서 랜덤한 포인트를 설정

	return Point;
}

void ASpawnVolume::SpawnOurPawn_Implementation(UClass* ToSpawn, const FVector& Location)	//BlueprintNativeEvent
{
	if (ToSpawn)
	{
		UWorld* World = GetWorld();//무언가를 스폰할 때 world 가 필요

		if (World)
		{
			FActorSpawnParameters SpawnParams;
			ACritter* CritterSpawned = World->SpawnActor<ACritter>(ToSpawn, Location, FRotator(0.f), SpawnParams);
		}
	}
}