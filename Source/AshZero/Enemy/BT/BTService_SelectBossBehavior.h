// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Enemy/AZBossAIController.h"
#include "BTService_SelectBossBehavior.generated.h"

/**
 * 보스 행동 결정 서비스
 *
 * 로직:
 * 1. 각도 체크 → Turn
 * 2. 스태미나 부족 → Strafe
 * 3. 스태미나 충분 → Attack (공격 선택 + 이동은 Task에서)
 */

class AAZBossBase;

UCLASS()
class ASHZERO_API UBTService_SelectBossBehavior : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SelectBossBehavior();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// ===== 설정 값 =====
	// 회전이 필요한 각도 (이 각도 이상이면 Turn)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float TurnAngleThreshold = 30.0f;

	// ===== 블랙보드 키 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector StaminaKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector DistanceToTargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector BehaviorKey;

private:
	// Strafe 상태 추적
	bool bIsCurrentlyStrafing = false;

	// Behavior 설정 + 속도 적용
	void SetBehaviorWithSpeed(AAZBossBase* Boss, UBlackboardComponent* BB, EBossBehavior NewBehavior);
};
























