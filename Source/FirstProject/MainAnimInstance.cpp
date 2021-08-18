// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Main.h"

void UMainAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();	// 폰의 소유자를 구함
		if (Pawn)
		{
			Main = Cast<AMain>(Pawn);	//Main과 Pawn이 같게 만듦 , Pawn을 Main에 캐스팅, 블루프린트에서 main을 만질수 있도록
		}
	}
}

void UMainAnimInstance::UpdateAnimationProperties()	//블루프린트로 연결
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			Main = Cast<AMain>(Pawn);
		}
	}

	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity();	// 캐릭터의 스피드를 얻고
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);	//우리가 관심있는건 앞뒤좌우의 속도이기 때문에 위아래를 뺀 횡방향벡터를 구함
		MovementSpeed = LateralSpeed.Size();	// .Size()로 float속도를 구함

		bIsInAir = Pawn->GetMovementComponent()->IsFalling();
	}
	
}