// Fill out your copyright notice in the Description page of Project Settings.


#include "Cpt_FootIK.h"
#include "Main.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

// Sets default values for this component's properties
UCpt_FootIK::UCpt_FootIK()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


    m_fIKFeetInterpSpeed = 22.0f;
    m_fIKTraceDistance = 55.0f;

    m_sIKSocketName_LeftFoot = "foot_l";
    m_sIKSocketName_RightFoot = "foot_r";
}


// Called when the game starts
void UCpt_FootIK::BeginPlay()
{
	Super::BeginPlay();

    m_pCharacter = Cast<AMain>(GetOwner());
    if (m_pCharacter == nullptr) return;

    m_fIKCapsuleHalkHeight = m_pCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

}

void UCpt_FootIK::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
    m_pCharacter = nullptr;

}

// Called every frame
void UCpt_FootIK::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ST_IK_TraceInfo pTrace_Left = IK_FootTrace(m_fIKTraceDistance, m_sIKSocketName_LeftFoot);
    ST_IK_TraceInfo pTrace_Right = IK_FootTrace(m_fIKTraceDistance, m_sIKSocketName_RightFoot);
    
    //! Update foot rotation from Impact normal
    if (IsMoving() == false)
    {
        IK_Update_FootRotation(DeltaTime, NormalToRotator(pTrace_Left.pImpactLocation), &m_stIKAnimValuse.m_pFootRotation_Left, m_fIKFeetInterpSpeed);
        IK_Update_FootRotation(DeltaTime, NormalToRotator(pTrace_Right.pImpactLocation), &m_stIKAnimValuse.m_pFootRotation_Right, m_fIKFeetInterpSpeed);
    }
    else
    {
        IK_Update_FootRotation(DeltaTime, FRotator::ZeroRotator, &m_stIKAnimValuse.m_pFootRotation_Left, m_fIKFeetInterpSpeed);
        IK_Update_FootRotation(DeltaTime, FRotator::ZeroRotator, &m_stIKAnimValuse.m_pFootRotation_Right, m_fIKFeetInterpSpeed);
    }

    // 양발에 LineTrace했던 데이터를 토대로 낮은 위치의 발 기준으로 Hip위치를 조절하고 CapsuleComponent위치를 조정한다.
    float fHipsOffset = UKismetMathLibrary::Min(pTrace_Left.fOffset, pTrace_Right.fOffset);
    if (fHipsOffset < 0.0f == false) fHipsOffset = 0.0f;
    IK_Update_FootOffset(DeltaTime, fHipsOffset, &m_stIKAnimValuse.m_fHipOffset, m_fIKFeetInterpSpeed);
    IK_Update_CapsuleHalfHeight(DeltaTime, fHipsOffset, false);

    //! Update effector locations of foot,  FootOffset 구하기
    IK_Update_FootOffset(DeltaTime, pTrace_Left.fOffset - fHipsOffset, &m_stIKAnimValuse.m_fEffectorLocation_Left, m_fIKFeetInterpSpeed);
    IK_Update_FootOffset(DeltaTime, -1 * (pTrace_Right.fOffset - fHipsOffset), &m_stIKAnimValuse.m_fEffectorLocation_Right, m_fIKFeetInterpSpeed);
}

ST_IK_TraceInfo UCpt_FootIK::IK_FootTrace(float fTraceDistance, FName sSocket)
{
    ST_IK_TraceInfo pTraceInfo;

    // Set Linetraces startpoint and end point
    FVector pSocketLocation = m_pCharacter->GetMesh()->GetSocketLocation(sSocket);
    FVector pLine_Start = FVector(pSocketLocation.X, pSocketLocation.Y, m_pCharacter->GetActorLocation().Z);
    FVector pLine_End = FVector(pSocketLocation.X, pSocketLocation.Y, (m_pCharacter->GetActorLocation().Z - m_fIKCapsuleHalkHeight) - fTraceDistance);

    // Process Line Trace
    FHitResult pHitResult;
    TArray<AActor*> pIgnore;
    pIgnore.Add(GetOwner());

    bool bDebug = true;
    EDrawDebugTrace::Type eDebug = EDrawDebugTrace::None;
    if (bDebug == true) eDebug = EDrawDebugTrace::ForOneFrame;

    bool bResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(), pLine_Start, pLine_End,
        UEngineTypes::ConvertToTraceType(ECC_Visibility), true, pIgnore, EDrawDebugTrace::None, pHitResult, true);

    // Set ImpactNormal and Offset from HitResult
    pTraceInfo.pImpactLocation = pHitResult.ImpactNormal;
    if (pHitResult.IsValidBlockingHit() == true)
    {
        float fImpactLegth = (pHitResult.ImpactPoint - pHitResult.TraceEnd).Size();
        pTraceInfo.fOffset = 0.0f + (fImpactLegth - fTraceDistance);    // 5.0f -> 0.0f 수정
    }
    else
    {
        pTraceInfo.fOffset = 0.0f;
    }

    return pTraceInfo;  //IK_FootTrace에서 반환 되는 구조체는 LineTrace의 HitPoint와 발, 땅의 Offset값이 들어있다.

}

FRotator UCpt_FootIK::NormalToRotator(FVector pVector)
{
    float fAtan2_1 = UKismetMathLibrary::DegAtan2(pVector.Y, pVector.Z);
    float fAtan2_2 = UKismetMathLibrary::DegAtan2(pVector.X, pVector.Z);
    fAtan2_2 *= -1.0f;
    FRotator pResult = FRotator(fAtan2_2, 0.0f, fAtan2_1);

    return pResult;     // IK_FootTrace함수에서 나온 땅위치 값 바탕으로 바닥 Normal Vector를 알 수 있다.
}

void UCpt_FootIK::IK_Update_FootRotation(float fDeltaTime, FRotator pTargetValue, FRotator* pFootRotatorValue, float fInterpSpeed)
{
    //! Set Foot Rotation value with FInterpTo
    FRotator pInterpRotator = UKismetMathLibrary::RInterpTo(*pFootRotatorValue, pTargetValue, fDeltaTime, fInterpSpeed);
    *pFootRotatorValue = pInterpRotator;
}

void UCpt_FootIK::IK_Update_CapsuleHalfHeight(float fDeltaTime, float fHipsShifts, bool bResetDefault)
{
    UCapsuleComponent* pCapsule = m_pCharacter->GetCapsuleComponent();
    if (pCapsule == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("IK : Capsule is NULL"));
        return;
    }

    //! Get Half Height of capsule component
    float fCapsuleHalf = 0.0f;
    if (bResetDefault == true)
    {
        fCapsuleHalf = m_fIKCapsuleHalkHeight;
    }
    else
    {
        float fHalfAbsSize = UKismetMathLibrary::Abs(fHipsShifts) * 0.5f;
        fCapsuleHalf = m_fIKCapsuleHalkHeight - fHalfAbsSize;
    }

    //! Set capsule component height with FInterpTo 
    float fScaledCapsuleHalfHeight = pCapsule->GetScaledCapsuleHalfHeight();
    float fInterpValue = UKismetMathLibrary::FInterpTo(fScaledCapsuleHalfHeight, fCapsuleHalf, fDeltaTime, 13.0f);

    pCapsule->SetCapsuleHalfHeight(fInterpValue, true);
}

void UCpt_FootIK::IK_Update_FootOffset(float fDeltaTime, float fTargetValue, float* fEffectorValue, float fInterpSpeed)
{
    float fInterpValue = UKismetMathLibrary::FInterpTo(*fEffectorValue, fTargetValue, fDeltaTime, fInterpSpeed);
    *fEffectorValue = fInterpValue;
}

bool UCpt_FootIK::IsMoving()
{
    float fSpeed = m_pCharacter->GetVelocity().Size();
    if (fSpeed > 0.0f) return true;
    return false;
}