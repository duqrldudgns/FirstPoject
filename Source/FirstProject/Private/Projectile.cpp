// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/ArrowComponent.h"

AProjectile::AProjectile()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 5000.f;
	ProjectileMovementComponent->MaxSpeed = 5000.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;

	CheckImpactLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("CheckImpactLocation"));
	CheckImpactLocation->SetupAttachment(Mesh);

	InitialLifeSpan = 10.f;

	GravityScale = 1.f;
}