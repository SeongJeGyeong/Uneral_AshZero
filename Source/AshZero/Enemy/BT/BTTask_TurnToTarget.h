// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TurnToTarget.generated.h"

/**
 * 보스가 타겟을 향해 Turn 애니메이션을 재생하면서 회전
 */
UCLASS()
class ASHZERO_API UBTTask_TurnToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TurnToTarget();

	// 타겟 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	// 회전 속도 (도/초)
	UPROPERTY(EditAnywhere, Category = "Turn")
	float TurnSpeed = 180.0f;

	// 회전 완료 판정 각도 (이 각도 이하면 완료)
	UPROPERTY(EditAnywhere, Category = "Turn")
	float AcceptanceAngle = 0.0f;

	// Turn 애니메이션 (왼쪽)
	UPROPERTY(EditAnywhere, Category = "Turn|Animation")
	UAnimMontage* TurnLeftMontage;

	// Turn 애니메이션 (오른쪽)
	UPROPERTY(EditAnywhere, Category = "Turn|Animation")
	UAnimMontage* TurnRightMontage;

	// 애니메이션 없이 회전만 할지
	UPROPERTY(EditAnywhere, Category = "Turn|Animation")
	bool bUseAnimationTurn = true;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;



private:
	// 타겟까지의 각도 계산 (양수: 오른쪽, 음수: 왼쪽)
	float GetAngleToTarget(class AAZBossBase* Boss, AActor* Target) const;

	UPROPERTY()
	class AAZBossBase* CachedBoss;

	bool bIsTurning = false;
};