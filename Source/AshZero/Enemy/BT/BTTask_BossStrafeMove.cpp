// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_BossStrafeMove.h"
#include "Enemy/AZBossBase.h"
#include "Enemy/AZBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"

UBTTask_BossStrafeMove::UBTTask_BossStrafeMove()
{
	NodeName = TEXT("Boss Strafe Move");
	bNotifyTick = true;
	bCreateNodeInstance = true;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_BossStrafeMove, TargetKey), AActor::StaticClass());
	BehaviorKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_BossStrafeMove, BehaviorKey), StaticEnum<EBossBehavior>());
}

EBTNodeResult::Type UBTTask_BossStrafeMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AAZBossBase* Boss = Cast<AAZBossBase>(AIController->GetPawn());
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));

	if (!Boss || !Target) return EBTNodeResult::Failed;

	// 1. 방향 결정 (왼쪽 1.0f 또는 오른쪽 -1.0f)
	float Direction = FMath::RandBool() ? 1.0f : -1.0f;

	// 2. 목적지 계산 (거리 상관없이 멀찍이 잡기)
	FVector BossLoc = Boss->GetActorLocation();
	FVector TargetLoc = Target->GetActorLocation();

	// 플레이어에서 보스를 향하는 방향 벡터
	FVector FromTargetToBoss = (BossLoc - TargetLoc).GetSafeNormal2D();

	// 보스의 측면 방향 계산 (90도 회전)
	FVector SideVector = FVector::CrossProduct(FVector::UpVector, FromTargetToBoss) * Direction;

	// 대각선 뒤쪽 방향으로 목적지를 잡아서 뒤로 물러나는 느낌을 줌
	FVector CombinedDir = (FromTargetToBoss + SideVector).GetSafeNormal();
	StrafeTargetLocation = TargetLoc + (CombinedDir * StrafeRadius);

	return EBTNodeResult::InProgress;
}

void UBTTask_BossStrafeMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AAZBossBase* Boss = Cast<AAZBossBase>(AIController->GetPawn());
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));

	if (!Boss || !Target)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 1. 타겟을 바라보기 (Strafe의 핵심: 몸은 가는데 시선은 고정)
	FVector LookDir = Target->GetActorLocation() - Boss->GetActorLocation();
	LookDir.Z = 0;
	Boss->SetActorRotation(LookDir.Rotation());

	// 2. 목표 지점으로 이동
	FVector MoveDir = StrafeTargetLocation - Boss->GetActorLocation();
	MoveDir.Z = 0;

	// 도착 여부 확인 (근사치)
	if (MoveDir.Size() < ArrivalThreshold)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	Boss->AddMovementInput(MoveDir.GetSafeNormal(), 1.0f);
}

EBTNodeResult::Type UBTTask_BossStrafeMove::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogTemp, Warning, TEXT("BossStrafeMove: Aborted"));
	return EBTNodeResult::Aborted;
}