// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AZAreaAttackNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class ASHZERO_API UAZAreaAttackNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UAZAreaAttackNotifyState();

	// ===== 공격 설정 =====
	UPROPERTY(EditAnywhere, Category = "AZ|Attack")
	FName SocketName = TEXT("Muzzle"); // 소켓

	UPROPERTY(EditAnywhere, Category = "AZ|Attack")
	float AttackRange = 800.0f; // 공격 거리

	UPROPERTY(EditAnywhere, Category = "AZ|Attack")
	float AttackAngle = 45.0f; // 부채꼴 각도 (중심으로부터 양옆 각도)

	// 높이 추적 여부
	UPROPERTY(EditAnywhere, Category = "AZ|Attack")
	bool bTargetHeightTracking = true;

	UPROPERTY(EditAnywhere, Category = "AZ|Attack")
	float TargetHeightOffset = 60.0f;

	// ===== 데미지 설정 =====
	UPROPERTY(EditAnywhere, Category = "AZ|Attack")
	float DamageInterval = 0.1f; // 틱 간격

	// ===== 효과 설정 =====
	UPROPERTY(EditAnywhere, Category = "AZ|VFX")
	class UNiagaraSystem* AreaVFX;

	UPROPERTY(EditAnywhere, Category = "AZ|VFX")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "AZ|VFX")
	bool bDebugDraw = false;

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	void PerformAreaTrace(USkeletalMeshComponent* MeshComp, float DeltaTime);

	UPROPERTY()
	class AAZBossBase* OwnerBoss;

	UPROPERTY()
	class UNiagaraComponent* SpawnedVFX;

	float TraceTimer = 0.0f;
};
