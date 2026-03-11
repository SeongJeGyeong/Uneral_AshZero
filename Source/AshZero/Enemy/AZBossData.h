// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AZBossData.generated.h"


/**
 * 개별 공격 패턴 데이터
 */
USTRUCT(BlueprintType)
struct FBossAttackData
{
	GENERATED_BODY()

	// 공격 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Attack")
	UAnimMontage* Montage = nullptr;

	// 공격 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Attack")
	float Damage = 20.0f;

	// 스태미나 소모량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Attack")
	float StaminaCost = 20.0f;

	// 발동 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Attack", meta = (ClampMin = "0", ClampMax = "100"))
	float Chance = 100.0f;

	// 공격 사거리 (이 거리 안에 들어와야 공격 실행)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Attack")
	float AttackRange = 300.0f;

	// 공격 타입 (이동 방식 결정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Attack")
	bool bNeedApproach = true;

	// 연속 사용 가능 여부 (true면 LastSuccessTag 체크를 무시함)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanRepeat = false;

};

/**
 * 페이즈별 설정 데이터
 */
USTRUCT(BlueprintType)
struct FBossPhaseData
{
	GENERATED_BODY()

	// 공격 가능 스태미나 임계값 (이 이상이면 공격 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Phase")
	float AttackThreshold = 35.0f;

	// Strafe 탈출 스태미나
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Phase")
	float StrafeExitThreshold = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Phase")
	TArray<int32> AvailablePatternIndices;

	// Approach 속도 (플레이어에게 접근할 때)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Phase")
	float ApproachSpeed = 300.0f;

	// Strafe 속도 (옆으로 이동할 때 관찰 느낌을 주기 위한)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Phase")
	float StrafeSpeed = 250.0f;

	// 체력 비율 임계값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Phase", meta = (ClampMin = "0", ClampMax = "1"))
	float HealthThreshold = 0.5f;
};














