// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AZEnemyBase.h"
#include "Enemy/AZBossData.h"
#include "AZGameplayTags.h"
#include "AZBossBase.generated.h"

class UBehaviorTree;
class UBlackboardData;
class AAZBossAIController;

// 보스 페이즈 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, int32, NewPhase);

// 패턴 종료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPatternFinished);

UCLASS()
class ASHZERO_API AAZBossBase : public AAZEnemyBase
{
	GENERATED_BODY()

public:
	AAZBossBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ===== 행동트리 =====
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|AI")
	UBehaviorTree* BossBehaviorTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|AI")
	UBlackboardData* BossBlackboard;

	// ===== 페이즈 시스템 =====
public:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase, BlueprintReadOnly, Category = "AZ|Phase")
	int32 CurrentPhase = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Phase")
	float PhaseTransitionInvincibleTime = 3.0f;

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Phase")
	bool bIsInPhaseTransition = false;

	UPROPERTY(BlueprintAssignable, Category = "AZ|Phase")
	FOnPhaseChanged OnPhaseChangedDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Phase")
	UAnimMontage* PhaseTransitionMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Phase")
	TArray<FBossPhaseData> PhaseSettings;

	UFUNCTION()
	void OnRep_CurrentPhase();

	UFUNCTION(BlueprintCallable, Category = "AZ|Phase")
	void CheckPhaseTransition();

	UFUNCTION(BlueprintNativeEvent, Category = "AZ|Phase")
	void OnPhaseChanged(int32 NewPhase);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPhaseTransition(int32 NewPhase);

	UFUNCTION(BlueprintImplementableEvent, Category = "AZ|Phase")
	void SetPhaseIndex(int32 Phase) const;

	// ===== 페이즈 관련 헬퍼 =====
	UFUNCTION(BlueprintPure, Category = "AZ|Phase")
	float GetCurrentAttackThreshold() const;

	UFUNCTION(BlueprintPure, Category = "AZ|Phase")
	float GetCurrentStrafeExitThreshold() const;

	UFUNCTION(BlueprintPure, Category = "AZ|Phase")
	float GetCurrentApproachSpeed() const;

	UFUNCTION(BlueprintPure, Category = "AZ|Phase")
	float GetCurrentStrafeSpeed() const;

	// ===== 공격 시스템 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Attack")
	TArray<FBossAttackData> AttackPatterns;

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Attack")
	int32 SelectedAttackIndex = -1;

	UFUNCTION(BlueprintCallable, Category = "AZ|Attack")
	int32 SelectAttackIndex();

	UFUNCTION(BlueprintCallable, Category = "AZ|Attack")
	bool PerformAttackByIndex(int32 AttackIndex);

	UFUNCTION(BlueprintPure, Category = "AZ|Attack")
	float GetCurrentAttackDamage() const;

	UFUNCTION(BlueprintPure, Category = "AZ|Attack")
	float GetAttackRange(int32 AttackIndex) const;

	UFUNCTION(BlueprintPure, Category = "AZ|Attack")
	bool DoesAttackNeedApproach(int32 AttackIndex) const;

	// ===== 스태미나 시스템 =====
public:
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "AZ|Stamina")
	float CurrentStamina;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Stamina")
	float MaxStamina = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Stamina")
	float StaminaRegenRate = 2.0f;

	UFUNCTION(BlueprintCallable, Category = "AZ|Stamina")
	void ConsumeStamina(float Amount);

	UFUNCTION(BlueprintPure, Category = "AZ|Stamina")
	bool HasEnoughStamina(float Amount) const;

	UFUNCTION(BlueprintPure, Category = "AZ|Stamina")
	float GetStaminaPercent() const;

protected:
	void RegenStamina(float DeltaTime);

	// ===== 패턴 실행 =====
public:
	UPROPERTY(BlueprintReadOnly, Category = "AZ|Pattern")
	bool bIsPerformingPattern = false;

	UPROPERTY(BlueprintAssignable, Category = "AZ|Pattern")
	FOnPatternFinished OnPatternFinishedDelegate;

	UFUNCTION(BlueprintCallable, Category = "AZ|Pattern")
	void OnPatternFinished();

	UFUNCTION()
	void OnPatternMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	// ===== 타겟 관련 =====
	UFUNCTION(BlueprintPure, Category = "AZ|Target")
	float GetDistanceToTarget() const;

	virtual AActor* GetCurrentTarget() const override;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Aggro")
	float AggroUpdateInterval = 10.0f;

	void OnPlayerDetected();
	void UpdateAggroTarget();
	void StartAggroTimer();
	void StopAggroTimer();

	bool IsValidTarget(AController* Controller) const;

	AController* FindAnyAlivePlayer() const;

protected:
	UPROPERTY()
	TMap<AController*, float> DamageHistory;

	UPROPERTY()
	AController* CurrentAggroController;

	FTimerHandle AggroUpdateTimerHandle;

	// ===== 이동 & 회전 =====
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|Movement")
	float IdleSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Movement")
	float TurnSpeed = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Movement")
	float TurnDirection = 0.0f;

protected:
	void RotateToTargetWithTurn(float DeltaTime);

	// ===== 사망 =====
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Death")
	TSubclassOf<AActor> PortalClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Death")
	FVector PortalSpawnOffset = FVector(0.f, 0.f, 50.f);

	virtual void OnDeath(FVector DeathLocation, AActor* Killer) override;

	// ===== 유틸리티 함수 =====
	UFUNCTION(BlueprintPure, Category = "AZ|Utility")
	AAZBossAIController* GetBossAIController() const;

	UFUNCTION(BlueprintCallable, Category = "AZ|Utility")
	void UpdateBlackboardValues();

	UFUNCTION(BlueprintPure, Category = "AZ|Utility")
	float GetHealthPercent() const;

protected:
	UFUNCTION()
	void OnBossTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Attack")
	int32 LastAttackIndex = -1;

private:
	FTimerHandle PhaseTransitionTimerHandle;
	void EndPhaseTransition();

	bool bBossUIActivated = false;

};