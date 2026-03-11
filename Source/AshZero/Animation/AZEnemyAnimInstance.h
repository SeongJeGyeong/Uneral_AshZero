// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Enemy/AZEnemyFSM.h"
#include "AZEnemyAnimInstance.generated.h"


UCLASS()
class ASHZERO_API UAZEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AZ|FSM")
	EEnemyState AnimState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AZ|FSM")
	float Speed = 0.0f;

	/*
	// AnimNotify에서 호출할 함수들
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnAttackHitCheck();  // 공격 판정 시점

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnAttackAnimEnd();    // 공격 애니 종료
	*/
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY()
	class AAZEnemyBase* OwnerEnemy;

};