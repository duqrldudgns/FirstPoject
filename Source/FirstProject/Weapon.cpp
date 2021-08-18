// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetRootComponent());

	bWeaponParticles = false;

	WeaponState = EWeaponState::EWS_Pickup;

	Damage = 25.f;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 충돌 유형 지정
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // QueryOnly : 물리학 계산 하지 않음
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);	// 모든Dynamic 요소에 대한 충돌
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	// 그 충돌에 대한 반응은 무시하고
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);	// Pawn에 대한 충돌만 Overlap으로 설정

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapEnd);

}


void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if ((WeaponState == EWeaponState::EWS_Pickup) && OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			Main->SetActiveOverlappingItem(this);
		}
	}
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			Main->SetActiveOverlappingItem(nullptr);
		}
	}
}

void AWeapon::Equip(AMain* Character)
{
	if (Character)
	{
		// 장비를 장착할 때 그 캐릭터의 컨트롤러를 세팅해야 누가 피해를 주는지 알 수 있고 피해를 줄 수 있음
		SetInstigator(Character->GetController());	

		// 카메라와 내 캐릭터에 대한 충돌 무시
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		SkeletalMesh->SetSimulatePhysics(false);	// 물리학 시뮬레이션을 하고 있다면 중단하는 것.. ??
	
		// "RightHandSocket" 이라는 이름의 소켓을 가져옴
		const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName("RightHandSocket"); 
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(this, Character->GetMesh());	// 그 캐릭터의 소켓에 이 무기를(this) 붙임
			bRotate = false;
			
			// 기존 장비 파괴
			Character->SetEquippedWeapon(this);	// Main에서 이 무기를 다룰 수 있게 함
			Character->SetActiveOverlappingItem(nullptr);	// 끄지 않으면 계속 overlap 처리 돼있음

			WeaponState = EWeaponState::EWS_Equipped;
		}

		if (OnEquipSound) UGameplayStatics::PlaySound2D(this, OnEquipSound);

		// 불속성 물속성 등 유지 할 것인지
		if (!bWeaponParticles) IdleParticlesComponent->Deactivate();	// 체크되어 있지 않으면 파티클 효과 끄기

	}
}

void AWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (Enemy->HitParticles)
			{
				// 스켈레톤에 들어가 설정해둔 소켓 이름인 "WeaponSocket"을 가져와 그 위치에 효과 발생		-	Enemy 설정과 비슷하지만 다르니 참고, 이건 SkeletalMesh를 미리 설정 해뒀기 때문에 가능
				const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
				if (WeaponSocket)
				{
					FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}
			if (Enemy->HitSound)	// 때렸을 때 Enemy에서 나는 소리
			{
				UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
			}
			if (DamageTypeClass)
			{
				// TakeDamage 함수와 연동되어 데미지를 입힘
				UGameplayStatics::ApplyDamage(Enemy, Damage, WeaponInstigator, this, DamageTypeClass);	// 피해대상, 피해량, 컨트롤러(가해자), 피해 유발자, 손상유형
			}
		}
	}
}

void AWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AWeapon::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // QueryOnly : 물리학 계산 하지 않음
}

void AWeapon::DeActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}