// Fill out your copyright notice in the Description page of Project Settings.


#include "ColliderMovementComponent.h"

void UColliderMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime)) 
	{
		return;
	}

	FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f);	//우리가 움직이고자 하는 움직임의 양 ,  벡터의 값을 반환한 다음 지움(0으로 만듦?) . 최대 1로 압축

	if (!DesiredMovementThisFrame.IsNearlyZero())	// ! 거의 0에 가까우면 = 0에가깝지않으면, 거의 0이아니라면
	{
		FHitResult Hit;
		
		//업데이트된 컴포넌트를 옮김 , desired 우리가 이 프레임을 얼마나 움직이고 싶은지를 보여주는 방향과 크기 , 업데이트에서 요소를 가져오고 만일에 대비하여 적중결과를 전달
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);		

		if (Hit.IsValidBlockingHit())	//무엇을 쳤다면 측면을따라 미끄러짐
		{
			SlideAlongSurface(DesiredMovementThisFrame, 1.f - Hit.Time, Hit.Normal, Hit);	//time : 충돌 후 남은시간,  normal : 표면을 쳤을때 치는 방향
			UE_LOG(LogTemp, Warning, TEXT("Valid Blocking Hit"));
		}
	}
}