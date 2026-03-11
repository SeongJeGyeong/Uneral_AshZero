// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "AZBossAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class AAZBossBase;

// 보스 행동 상태
UENUM(BlueprintType)
enum class EBossBehavior : uint8
{
	Idle = 0,
	Turn = 1,
	Strafe = 2,
	Attack = 3
};

UCLASS()
class ASHZERO_API AAZBossAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAZBossAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	// ===== 컴포넌트 =====
	UPROPERTY()
	UBehaviorTreeComponent* BehaviorTreeComp;

	UPROPERTY()
	UBlackboardComponent* BlackboardComp;

	UPROPERTY()
	AAZBossBase* ControlledBoss;

	// ===== 블랙보드 접근 함수 =====
public:	
	UFUNCTION(BlueprintCallable, Category = "AZ|Boss")
	void SetBlackboardTarget(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "AZ|Boss")
	AActor* GetBlackboardTarget() const;

	UFUNCTION(BlueprintCallable, Category = "AZ|Boss")
	void SetBehavior(EBossBehavior NewBehavior);

	UFUNCTION(BlueprintPure, Category = "AZ|Boss")
	EBossBehavior GetCurrentBehavior() const;

	UFUNCTION(BlueprintPure, Category = "AZ|Boss")
	float GetBlackboardFloatValue(FName KeyName) const;

	UFUNCTION(BlueprintPure, Category = "AZ|Boss")
	int32 GetBlackboardIntValue(FName KeyName) const;

	// ===== 블랙보드 키 =====
public:
	static const FName BBKey_Target;
	static const FName BBKey_CanSeePlayer;
	static const FName BBKey_LastKnownLocation;
	static const FName BBKey_CurrentPhase;
	static const FName BBKey_Stamina;
	static const FName BBKey_DistanceToTarget;
	static const FName BBKey_Behavior;
	static const FName BBKey_IsPerformingPattern;


};