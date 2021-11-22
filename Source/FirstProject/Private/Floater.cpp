// Fill out your copyright notice in the Description page of Project Settings.


#include "Floater.h"

// Sets default values
AFloater::AFloater()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CustomStaticMesh"));

	InitialLocation = FVector(0.0f);
	PlacedLocation = FVector(0.0f);
	WorldOrigin = FVector(0.0f);
	InitialDirection = FVector(0.0f);

	bInitializeFloaterLocations = false;
	bShouldFloat = false;

	InitialForce = FVector(2000000.f, 0.0f, 0.0f);
	InitialTorque = FVector(2000000.f, 0.0f, 0.0f);

	RunningTime = 0.f;
}

// Called when the game starts or when spawned
void AFloater::BeginPlay()
{
	Super::BeginPlay();

	//float InitialX = FMath::FRand();	
	//float InitialY = FMath::FRand();
	//float InitialZ = FMath::FRand();

	float InitialX = FMath::FRandRange(-500.f, 500.f);
	float InitialY = FMath::FRandRange(0.f, 500.f);
	float InitialZ = FMath::FRandRange(-500.f, 0.f);

	InitialLocation.X = InitialX;
	InitialLocation.Y = InitialY;
	InitialLocation.Z = InitialZ;

	//InitialLocation *= 500.f;

	PlacedLocation = GetActorLocation();

	if (bInitializeFloaterLocations)
	{
		SetActorLocation(InitialLocation);
	}

	//FHitResult HitResult;
	//FVector LocalOffset = FVector(1000.f, 0.0f, 0.0f);
	//AddActorWorldOffset(LocalOffset, true, &HitResult);	//World

	//StaticMesh->AddForce(InitialForce);		//힘
	//StaticMesh->AddTorque(InitialTorque);	//회전
	
	BaseZLocation = PlacedLocation.Z;
}

// Called every frame
void AFloater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldFloat)
	{
		/*
		FHitResult HitResult;
		AddActorLocalOffset(InitialDirection, true, &HitResult);	//sweep 충돌처리

		FVector HitLocation = HitResult.Location;

		UE_LOG(LogTemp, Warning, TEXT("Hit Location : X = %f, Y = %f, Z = %f"), HitLocation.X, HitLocation.Y, HitLocation.Z);
		*/

		FVector NewLocation = GetActorLocation();

		NewLocation.X = RunningTime * 100.f;
		NewLocation.Z = BaseZLocation + A * FMath::Sin(B * RunningTime - C) + D;

		SetActorLocation(NewLocation);

		RunningTime += DeltaTime;

	}

	//FRotator Rotation = FRotator(0.0f, 1.0f, 0.0f);
	//AddActorLocalRotation(Rotation);					//Local

}

