// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cpt_FootIK.generated.h"

typedef struct ST_IK_TraceInfo
{
	float	fOffset;			
	FVector pImpactLocation;	
};

//! IK Anim Instance Value struct
USTRUCT(Atomic, BlueprintType)
struct FST_IK_AnimValue
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float		m_fEffectorLocation_Left;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float		m_fEffectorLocation_Right;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator	m_pFootRotation_Left;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator	m_pFootRotation_Right;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float		m_fHipOffset;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FIRSTPROJECT_API UCpt_FootIK : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCpt_FootIK();

	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//! Leg joing move speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values")
	float m_fIKFeetInterpSpeed;
	
	//! Max Linetrace distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values")
	float m_fIKTraceDistance;

	//! Left foot bone name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values Socket")
	FName m_sIKSocketName_LeftFoot;
	//! Right foot bone name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values Socket")
	FName m_sIKSocketName_RightFoot;

	//! IK Anim Instance Value struct
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Values")
	FST_IK_AnimValue m_stIKAnimValuse;

	FORCEINLINE FST_IK_AnimValue GetIKAnimValue() { return m_stIKAnimValuse; }

private:

	// IK를 하기 위해서는 우선 발 밑으로 LineTrace를 하여 땅위치, Normal Vector를 알아내는 함수
	ST_IK_TraceInfo IK_FootTrace(float fTraceDistance, FName sSocket);
	
	UPROPERTY()
	class AMain* m_pCharacter;

	// Owner Characters capsule height
	float m_fIKCapsuleHalkHeight;

	//! Get ground normal,  IK_FootTrace함수에서 나온 땅위치 값 바탕으로 바닥 Normal Vector를 알 수 있다.
	FRotator NormalToRotator(FVector pVector);

	//! Process IK Foot rotation from ground normal,  NormalToRotator에서 나온 값을 RInterpTo함수를 이용해 값이 보간하며 변하게 하는 함수이다.
	void IK_Update_FootRotation(float fDeltaTime, FRotator pTargetValue, FRotator* pFootRotatorValue, float fInterpSpeed);

	//! Process IK characters capsule component Z Offset,  발의 높이에 따라 Capsule Collider 위치를 조절
	void IK_Update_CapsuleHalfHeight(float fDeltaTime, float fHipsShifts, bool bResetDefault);

	//! process IK Foot,leg Z Offset,    Vector값을 보간한다.  이 함수를 통해서 발 위치, Hip위치를 조절한다.
	void IK_Update_FootOffset(float fDeltaTime, float fTargetValue, float* fEffectorValue, float fInterpSpeed);

	//! Check owner character is moving
	bool IsMoving();
};

