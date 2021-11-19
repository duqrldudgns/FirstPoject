#include "BTService_Detect.h"
#include "EnemyAIController.h"
#include "Main.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Enemy.h"
#include "BossSevarogAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BossSevarog.h"

UBTService_Detect::UBTService_Detect()
{
	NodeName = TEXT("Detect");
	Interval = 0.5f;
}

void UBTService_Detect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AEnemy* ControllingPawn = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (nullptr == ControllingPawn) return;
	ABossSevarog* Boss = Cast<ABossSevarog>(OwnerComp.GetAIOwner()->GetPawn());

	UWorld* World = ControllingPawn->GetWorld();
	if (nullptr == World) return;

	FVector Center = ControllingPawn->GetActorLocation();

	// 구체를 만들어서 오브젝트를 감지한다.
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParam(NAME_None, false, ControllingPawn);
	bool bResult = World->OverlapMultiByChannel(
		OverlapResults,
		Center,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel4,
		FCollisionShape::MakeSphere(ControllingPawn->DetectRadius),
		CollisionQueryParam
	);
	//UE_LOG(LogTemp, Warning, TEXT("%d"), bResult);
	// 오브젝트가 감지가 되면, 그 오브젝트가 Main인지 검사한다.
	if (bResult)
	{
		for (FOverlapResult OverlapResult : OverlapResults)
		{
			//DrawDebugLine(World, ControllingPawn->GetActorLocation(), OverlapResult.GetActor()->GetActorLocation(), FColor::Red, false, 1.f);

			AMain* Main = Cast<AMain>(OverlapResult.GetActor());
			if (Main/* && Main->GetController()->IsPlayerController()*/)	//플레이어 컨트롤러인지 파악
			{
				FVector TargetLoc = Main->GetFloor();
				//UE_LOG(LogTemp, Warning, TEXT("%s"), *TargetLoc.ToString());
				// Main면, 블랙보드에 저장한다.
				OwnerComp.GetBlackboardComponent()->SetValueAsObject(AEnemyAIController::TargetKey, Main);
				OwnerComp.GetBlackboardComponent()->SetValueAsVector(ABossSevarogAIController::TargetLocKey, TargetLoc);
				if (nullptr == Boss) ControllingPawn->GetCharacterMovement()->MaxWalkSpeed = 400.f;

				// 디버깅 용.
				//DrawDebugSphere(World, Center, ControllingPawn->DetectRadius, 16, FColor::Green, false, 0.2f);
				//DrawDebugPoint(World, Main->GetActorLocation(), 10.0f, FColor::Blue, false, 0.2f);
				//DrawDebugLine(World, ControllingPawn->GetActorLocation(), Main->GetActorLocation(), FColor::Blue, false, 0.2f);
				return;
			}
		}
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(AEnemyAIController::TargetKey, nullptr);
		if (nullptr == Boss) ControllingPawn->GetCharacterMovement()->MaxWalkSpeed = 200.f;
	}

	//DrawDebugSphere(World, Center, ControllingPawn->DetectRadius, 16, FColor::Red, false, 0.2f);
	/*
	1. Selector는 자식 중 하나가 성공할 때까지, 왼쪽 노드에서 오른쪽 노드로 실행한다.
	2. 왼쪽 시퀀스 실행. Target이 없으므로 실패.
	3. 오른쪽 시퀀스 실행. 무난히 성공.
	4. 오른쪽 시퀀스 쪽의 노드가 끝이나면, 다시 1번부터 실행한다.
	5. 왼쪽 시퀀스 실행. Target이 있으므로 성공. 왼쪽 노드를 실행한다. 
	*/
}
