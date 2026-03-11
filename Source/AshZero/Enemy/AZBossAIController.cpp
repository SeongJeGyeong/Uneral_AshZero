// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/AZBossAIController.h"
#include "Enemy/AZBossBase.h"
#include "Character/AZPlayerCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AshZero.h"

// 블랙보드 키 정의
const FName AAZBossAIController::BBKey_Target = TEXT("Target");
const FName AAZBossAIController::BBKey_CanSeePlayer = TEXT("CanSeePlayer");
const FName AAZBossAIController::BBKey_LastKnownLocation = TEXT("LastKnownLocation");
const FName AAZBossAIController::BBKey_CurrentPhase = TEXT("CurrentPhase");
const FName AAZBossAIController::BBKey_Stamina = TEXT("Stamina");
const FName AAZBossAIController::BBKey_DistanceToTarget = TEXT("DistanceToTarget");
const FName AAZBossAIController::BBKey_Behavior = TEXT("Behavior");
const FName AAZBossAIController::BBKey_IsPerformingPattern = TEXT("IsPerformingPattern");

AAZBossAIController::AAZBossAIController()
{
	// 컴포넌트 생성
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

}

void AAZBossAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AAZBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 보스 캐스팅
	ControlledBoss = Cast<AAZBossBase>(InPawn);
	if (!ControlledBoss)
	{
		PRINT_LOG(TEXT("BossAIController: Failed to cast to AAZBossBase!"));
		return;
	}

	// 블랙보드 초기화
	if (ControlledBoss->BossBlackboard)
	{
		UseBlackboard(ControlledBoss->BossBlackboard, BlackboardComp);
	}

	// 행동트리 실행
	if (ControlledBoss->BossBehaviorTree)
	{
		RunBehaviorTree(ControlledBoss->BossBehaviorTree);
		PRINT_LOG(TEXT("BossAIController: BehaviorTree Started"));
	}
	else
	{
		PRINT_LOG(TEXT("BossAIController: No BehaviorTree assigned!"));
	}
}
void AAZBossAIController::OnUnPossess()
{
	Super::OnUnPossess();

	// 행동트리 정지
	if (BehaviorTreeComp)
	{
		BehaviorTreeComp->StopTree();
	}

	ControlledBoss = nullptr;
}

// ===== 블랙보드 접근 함수 =====
void AAZBossAIController::SetBlackboardTarget(AActor* Target)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(BBKey_Target, Target);
	}
}

AActor* AAZBossAIController::GetBlackboardTarget() const
{
	if (BlackboardComp)
	{
		return Cast<AActor>(BlackboardComp->GetValueAsObject(BBKey_Target));
	}
	return nullptr;
}

void AAZBossAIController::SetBehavior(EBossBehavior NewBehavior)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsEnum(BBKey_Behavior, static_cast<uint8>(NewBehavior));
	}
}

EBossBehavior AAZBossAIController::GetCurrentBehavior() const
{
	if (BlackboardComp)
	{
		return static_cast<EBossBehavior>(BlackboardComp->GetValueAsEnum(BBKey_Behavior));
	}
	return EBossBehavior::Idle;
}

float AAZBossAIController::GetBlackboardFloatValue(FName KeyName) const
{
	if (BlackboardComp)
	{
		return BlackboardComp->GetValueAsFloat(KeyName);
	}
	return 0.0f;
}

int32 AAZBossAIController::GetBlackboardIntValue(FName KeyName) const
{
	if (BlackboardComp)
	{
		return BlackboardComp->GetValueAsInt(KeyName);
	}
	return 0;
}