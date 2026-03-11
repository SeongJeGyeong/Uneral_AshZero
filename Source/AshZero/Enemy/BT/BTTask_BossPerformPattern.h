// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "Enemy/AZBossAIController.h"
#include "BTTask_BossPerformPattern.generated.h"

class AAZBossBase;

/**
 * 보스 공격 Task
 *
 * 로직:
 * 1. 공격 선택 (SelectedAttackIndex)
 * 2. bNeedApproach == true (근접) → 사거리까지 이동 후 공격
 * 3. bNeedApproach == false (대시/원거리) → 바로 공격
 * 4. 공격 완료 대기
 */
UCLASS()
class ASHZERO_API UBTTask_BossPerformPattern : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossPerformPattern();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
	// 블랙보드 키
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector BehaviorKey;

	// 사거리 여유 (사거리보다 이만큼 더 가까이)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float RangeBuffer = 50.0f;

private:
	// 현재 BT 컴포넌트 캐시
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;

	// 현재 상태
	enum class EAttackState : uint8
	{
		SelectAttack,     // 공격 선택
		Approaching,      // 이동 중
		Attacking,        // 공격 중
		Finished          // 완료
	};
	EAttackState CurrentState = EAttackState::SelectAttack;

	// 선택된 공격 정보 캐시
	int32 CachedAttackIndex = -1;
	float CachedAttackRange = 0.0f;
	bool bCachedNeedApproach = true;
};