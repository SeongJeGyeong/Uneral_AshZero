// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Enemy/AZBossAIController.h"
#include "BTTask_BossStrafeMove.generated.h"

/**
 * 보스 Strafe 이동 Task
 *
 * 플레이어 주변을 좌우로 이동하며 거리 유지
 */
UCLASS()
class ASHZERO_API UBTTask_BossStrafeMove : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossStrafeMove();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "AZ|Strafe")
	float StrafeRadius = 800.0f;	//Strafe 거리 보스 크기에 따라 조정

	UPROPERTY(EditAnywhere, Category = "AZ|Strafe")
	float ArrivalThreshold = 50.0f;	//Strafe 목적지 도착 임계값, 보스 크기에 따라 조정

protected:
	// 블랙보드 키
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Blackboard")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Blackboard")
	FBlackboardKeySelector BehaviorKey;

private:
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;

	// Strafe 방향 (1 = 오른쪽, -1 = 왼쪽)
	int32 StrafeDirection = 1;

	FVector StrafeTargetLocation; // 이번 이동의 목적지
	bool bInitialized = false;

	// 방향 전환 타이머
	float DirectionTimer = 0.0f;
};