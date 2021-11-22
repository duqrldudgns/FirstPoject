// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionDecal.h"
#include "Components/DecalComponent.h"

// Sets default values
APositionDecal::APositionDecal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PositionDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("PositionDecal"));
	PositionDecal->SetupAttachment(GetRootComponent());
	PositionDecal->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	static ConstructorHelpers::FObjectFinder<UMaterial> Decal(TEXT("Material'/Game/GameplayMechanics/test/CharaSelectDecal/CharSelectDecal_MT.CharSelectDecal_MT'"));
	if (Decal.Succeeded())
	{
		PositionDecal->SetDecalMaterial(Decal.Object);
	}

	SetRootComponent(PositionDecal);

	InitialLifeSpan = 1.f;
}

// Called when the game starts or when spawned
void APositionDecal::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APositionDecal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

