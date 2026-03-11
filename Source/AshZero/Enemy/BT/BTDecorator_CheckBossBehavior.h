// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "Enemy/AZBossAIController.h"
#include "BTDecorator_CheckBossBehavior.generated.h"


/**
 * 현재 Behavior가 지정된 값과 일치하는지 확인하는 데코레이터
 * Flow Abort Mode를 Self로 설정하면 Behavior 변경 시 즉시 분기 전환
 */
UCLASS()
class ASHZERO_API UBTDecorator_CheckBossBehavior : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckBossBehavior();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

public:
	// 체크할 Behavior 값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Behavior")
	EBossBehavior ExpectedBehavior = EBossBehavior::Idle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Blackboard")
	FBlackboardKeySelector BehaviorKey;
};
























