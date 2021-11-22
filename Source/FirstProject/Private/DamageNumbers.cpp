// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageNumbers.h"
#include "Components/WidgetComponent.h"

// Sets default values
ADamageNumbers::ADamageNumbers()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DamageWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	SetRootComponent(DamageWidget);
	DamageWidget->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidget->SetDrawSize(FVector2D(200.f, 200.f));

	InitialLifeSpan = 2.f;
}

// Called when the game starts or when spawned
void ADamageNumbers::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADamageNumbers::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

