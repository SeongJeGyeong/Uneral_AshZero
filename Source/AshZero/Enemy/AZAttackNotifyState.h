// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Util/AZDefine.h"
#include "AZAttackNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class ASHZERO_API UAZAttackNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	

public:
    UAZAttackNotifyState();

    // 공격 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float AttackRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    FName AttackSocketName = TEXT("hand_r");  // 공격 시작점 소켓

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    bool bDebugDraw = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    EHitReactionType HitReactionType = EHitReactionType::LightHit;

    // 넉백 힘 (Knockback일 때만 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (EditCondition = "HitReactionType == EHitReactionType::Knockback"))
    float KnockbackForce = 800.0f;

protected:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
    // 이미 맞은 액터들 (중복 방지)
    TArray<AActor*> AlreadyHitActors;

    // 캐싱용
    UPROPERTY()
    class AAZEnemyBase* OwnerEnemy;

    void PerformAttackCheck(USkeletalMeshComponent* MeshComp);
};
