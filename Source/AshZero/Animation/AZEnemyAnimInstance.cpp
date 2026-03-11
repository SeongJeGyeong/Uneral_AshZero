// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AZEnemyAnimInstance.h"
#include "Enemy/AZEnemyBase.h"
#include "Enemy/AZEnemyFSM.h"


void UAZEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerEnemy)
    {
        OwnerEnemy = Cast<AAZEnemyBase>(TryGetPawnOwner());
    }

    if (OwnerEnemy)
    {
        // FSM 상태 동기화
        if (UAZEnemyFSM* FSM = OwnerEnemy->FindComponentByClass<UAZEnemyFSM>())
        {
            AnimState = FSM->State;
        }

        // 이동 속도
        Speed = OwnerEnemy->GetVelocity().Size();
    }
}
/*
void UAZEnemyAnimInstance::OnAttackHitCheck()
{
    if (OwnerEnemy)
    {
        OwnerEnemy->OnAttackHit();
    }
}

void UAZEnemyAnimInstance::OnAttackAnimEnd()
{
    if (OwnerEnemy)
    {
        OwnerEnemy->OnAttackEnd();
    }
}*/