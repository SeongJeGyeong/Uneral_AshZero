// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_BossPerformPattern.h"
#include "Enemy/AZBossBase.h"
#include "Enemy/AZBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_BossPerformPattern::UBTTask_BossPerformPattern()
{
	NodeName = TEXT("Boss Perform Pattern");
	bNotifyTick = true;
	bCreateNodeInstance = true;

	RangeBuffer = 50.0f;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_BossPerformPattern, TargetKey), AActor::StaticClass());
	BehaviorKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_BossPerformPattern, BehaviorKey), StaticEnum<EBossBehavior>());
}

EBTNodeResult::Type UBTTask_BossPerformPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = &OwnerComp;
	CurrentState = EAttackState::SelectAttack;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AAZBossBase* Boss = Cast<AAZBossBase>(AIController->GetPawn());
	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	// 이미 패턴 수행 중이면 대기
	if (Boss->bIsPerformingPattern)
	{
		CurrentState = EAttackState::Attacking;
		return EBTNodeResult::InProgress;
	}

	// ========== 1. 공격 선택 ==========
	if (Boss->SelectedAttackIndex < 0)
	{
		Boss->SelectAttackIndex();
	}

	CachedAttackIndex = Boss->SelectedAttackIndex;
	if (CachedAttackIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossPerformPattern: No valid attack selected"));
		return EBTNodeResult::Failed;
	}

	// 공격 정보 캐시
	CachedAttackRange = Boss->GetAttackRange(CachedAttackIndex) - RangeBuffer;
	bCachedNeedApproach = Boss->DoesAttackNeedApproach(CachedAttackIndex);

	UE_LOG(LogTemp, Warning, TEXT("BossPerformPattern: Selected Attack%d (Range: %.1f, NeedApproach: %s)"),
		CachedAttackIndex + 1, CachedAttackRange, bCachedNeedApproach ? TEXT("Yes") : TEXT("No"));

	// ========== 2. 접근 필요 여부 결정 ==========
	if (bCachedNeedApproach)
	{
		// 근접 공격 → 사거리 체크 후 이동 or 공격
		CurrentState = EAttackState::Approaching;
	}
	else
	{
		// 대시/원거리 공격 → 바로 공격
		CurrentState = EAttackState::Attacking;

		bool bSuccess = Boss->PerformAttackByIndex(CachedAttackIndex);
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("BossPerformPattern: Failed to perform attack"));
			return EBTNodeResult::Failed;
		}
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_BossPerformPattern::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAZBossBase* Boss = Cast<AAZBossBase>(AIController->GetPawn());
	if (!Boss)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Behavior가 Attack이 아니면 중단
	EBossBehavior CurrentBehavior = static_cast<EBossBehavior>(BB->GetValueAsEnum(BehaviorKey.SelectedKeyName));
	if (CurrentBehavior != EBossBehavior::Attack && !Boss->bIsPerformingPattern && CurrentState != EAttackState::Approaching)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossPerformPattern: Interrupted! Behavior changed to %d"), static_cast<int32>(CurrentBehavior));
		Boss->SelectedAttackIndex = -1;  // 공격 선택 초기화
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 타겟 가져오기
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));

	switch (CurrentState)
	{
	case EAttackState::Approaching:
	{
		if (!Target)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		// 거리 체크
		float Distance = FVector::Dist(Boss->GetActorLocation(), Target->GetActorLocation());

		// 사거리 안에 들어왔으면 → 공격
		if (Distance <= CachedAttackRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("BossPerformPattern: In range! (Dist: %.1f <= %.1f) -> Attack!"),
				Distance, CachedAttackRange);

			bool bSuccess = Boss->PerformAttackByIndex(CachedAttackIndex);
			if (bSuccess)
			{
				CurrentState = EAttackState::Attacking;
			}
			else
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			}
		}
		else
		{
			// 타겟 방향으로 이동
			FVector DirectionToTarget = Target->GetActorLocation() - Boss->GetActorLocation();
			DirectionToTarget.Z = 0.0f;
			DirectionToTarget.Normalize();

			Boss->AddMovementInput(DirectionToTarget, 1.0f);

			// 이동 중에도 타겟 바라보기
			FRotator LookAtRotation = DirectionToTarget.Rotation();
			Boss->SetActorRotation(FMath::RInterpTo(Boss->GetActorRotation(), LookAtRotation, DeltaSeconds, 5.0f));
		}
	}
	break;

	case EAttackState::Attacking:
	{
		// 공격 완료 대기
		if (!Boss->bIsPerformingPattern)
		{
			UE_LOG(LogTemp, Warning, TEXT("BossPerformPattern: Attack finished!"));
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
	break;

	default:
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		break;
	}
}

EBTNodeResult::Type UBTTask_BossPerformPattern::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AAZBossBase* Boss = Cast<AAZBossBase>(AIController->GetPawn());
		if (Boss)
		{
			Boss->OnPatternFinishedDelegate.RemoveAll(this);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("BossPerformPattern: Aborted"));
	return EBTNodeResult::Aborted;
}

void UBTTask_BossPerformPattern::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AAZBossBase* Boss = Cast<AAZBossBase>(AIController->GetPawn());
		if (Boss)
		{
			Boss->OnPatternFinishedDelegate.RemoveAll(this);
		}
	}

	// 상태 초기화
	CurrentState = EAttackState::SelectAttack;
	CachedAttackIndex = -1;
}