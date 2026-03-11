// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NiagaraComponent.h"
#include "AZLaserNotifyState.generated.h"

class USplineComponent;

/**
 * 
 */
UCLASS()
class ASHZERO_API UAZLaserNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UAZLaserNotifyState();

	// ===== 레이저 설정 =====

	// 레이저 시작 소켓
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	FName LaserSocketName = TEXT("Head");

	// 레이저 최대 사거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	float MaxLaserRange = 4000.0f;

	// 레이저 반경 (굵기) - Sphere Trace용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	float LaserRadius = 30.0f;

	// ===== 데미지 설정 =====

	// 초당 데미지 (틱 데미지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	float DamagePerSecond = 50.0f;

	// 데미지 적용 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	float DamageInterval = 0.1f;

	// ===== 나이아가라 설정 =====

	// 빔 이펙트 (스플라인 기반 나이아가라)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	class UNiagaraSystem* BeamEffect;

	// 스플라인 기반 VFX 블루프린트 (BP_SplineVFX_MiasmaBoli)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	TSubclassOf<AActor> BeamVFXClass;

	// 나이아가라 시작점 파라미터 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	FName BeamStartParamName = TEXT("BeamStart");

	// 나이아가라 끝점 파라미터 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	FName BeamEndParamName = TEXT("BeamEnd");

	// 충돌 지점 이펙트 (옵션)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	class UNiagaraSystem* ImpactEffect;

	// ===== 디버그 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	bool bDebugDraw = false;

	// ===== 추적 설정 =====

	// true: 시작 시 방향 고정 / false: 타겟 추적
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	bool bFixDirectionAtStart = true;

	// 추적 시 회전 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser", meta = (EditCondition = "!bFixDirectionAtStart"))
	float TrackingRotationSpeed = 2.0f;

	// ===== 높이 설정 =====

// 플레이어 가슴 높이 오프셋 (캡슐 절반 높이 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Laser")
	float TargetHeightOffset = 60.0f;

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("Boss Laser"); }

private:
	// 레이저 발사 및 데미지 처리
	void PerformLaserTrace(USkeletalMeshComponent* MeshComp, class AAZBossBase* Boss, float DeltaTime);

	// 나이아가라 업데이트
	void UpdateBeamEffect(const FVector& Start, const FVector& End);

	// 캐싱
	UPROPERTY()
	class AAZBossBase* OwnerBoss;

	// 스폰된 VFX 액터
	UPROPERTY()
	AActor* BeamVFXActor;

	// VFX 내부 스플라인 컴포넌트
	UPROPERTY()
	USplineComponent* BeamSpline;

	UPROPERTY()
	UNiagaraComponent* BeamComponent;

	UPROPERTY()
	UNiagaraComponent* ImpactComponent;

	// 레이저 목표 높이 (발사 시점에 고정)
	float TargetLaserHeight = 0.0f;

	// 데미지 타이머
	float DamageTimer = 0.0f;

	// 이미 이번 틱에 데미지 준 액터들
	TSet<AActor*> DamagedActorsThisTick;
};
