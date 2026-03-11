// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AZAttackNotifyState.h"
#include "Enemy/AZEnemyBase.h"
#include "Enemy/AZEnemyFSM.h"
#include "Enemy/AZBossBase.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Util/AZDefine.h"
#include "Enemy/AZDamageType.h"
#include "AshZero.h"

UAZAttackNotifyState::UAZAttackNotifyState()
{
    // 기본값 설정
    AttackRadius = 50.0f;
    AttackRange = 150.0f;
}

void UAZAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    // 공격 시작 - 초기화
    AlreadyHitActors.Empty();
    //UE_LOG(LogTemp, Error, TEXT("=== AttackNotifyState BEGIN ==="));
    if (MeshComp && MeshComp->GetOwner())
    {
        OwnerEnemy = Cast<AAZEnemyBase>(MeshComp->GetOwner());
        //UE_LOG(LogTemp, Error, TEXT("Owner: %s"), OwnerEnemy ? *OwnerEnemy->GetName() : TEXT("NULL")); 
    }
}

void UAZAttackNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
 
    // 매 프레임 공격 체크
    if (!OwnerEnemy || !OwnerEnemy->HasAuthority()) return;
    if (OwnerEnemy->bIsDead) return;
    PerformAttackCheck(MeshComp);
}

void UAZAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    // 공격 종료 - 정리
    AlreadyHitActors.Empty();
    OwnerEnemy = nullptr;

    //PRINT_LOG(TEXT("Attack NotifyState End"));
}

void UAZAttackNotifyState::PerformAttackCheck(USkeletalMeshComponent* MeshComp)
{
    if (!OwnerEnemy || !MeshComp) return;
    if (!OwnerEnemy->HasAuthority()) return;
    if (OwnerEnemy->bIsDead) return;
    // 공격 시작점 (소켓 위치)
    FVector AttackOrigin = MeshComp->GetSocketLocation(AttackSocketName);

    // 공격 방향 (캐릭터 전방)
    FVector AttackDirection = OwnerEnemy->GetActorForwardVector();
    FVector AttackEnd = AttackOrigin + (AttackDirection * AttackRange);

    // 충돌 체크
    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerEnemy);
    QueryParams.bTraceComplex = false;

    bool bHit = MeshComp->GetWorld()->SweepMultiByChannel(
        HitResults,
        AttackOrigin,
        AttackEnd,
        FQuat::Identity,
        //ECC_GameTraceChannel1,  // 전투 채널
		ECC_Pawn,  // 플레이어 캐릭터와 충돌
        FCollisionShape::MakeSphere(AttackRadius),
        QueryParams
    );
   
    if (bDebugDraw)
    {
        DrawDebugSphere(MeshComp->GetWorld(), AttackOrigin, AttackRadius,12, FColor::Red, false, 10.f, 0, 4);
        DrawDebugLine(MeshComp->GetWorld(), AttackOrigin, AttackEnd, FColor::Red, false, 10.f, 0, 4);
    }

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (AlreadyHitActors.Contains(HitActor)) continue;

            if (AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(HitActor))
            {
                float Damage = 10.0f;
                if (AAZBossBase* Boss = Cast<AAZBossBase>(OwnerEnemy))
                {
                    Damage = Boss->GetCurrentAttackDamage();
                }
                else if (UAZEnemyFSM* FSM = OwnerEnemy->FindComponentByClass<UAZEnemyFSM>())
                {
                    Damage = FSM->Damage;
                }

				// 넉백 방향 계산
                FVector KnockbackDir = (Player->GetActorLocation() - OwnerEnemy->GetActorLocation()).GetSafeNormal();
                KnockbackDir.Z = 0.3f;  // 살짝 위로
                KnockbackDir.Normalize();

                TSubclassOf<UDamageType> DamageTypeClass = UAZDamageType::StaticClass();

                //  DamageType 생성
                UAZDamageType* DamageType = NewObject<UAZDamageType>();
                DamageType->HitReactionType = HitReactionType;
                DamageType->KnockbackForce = KnockbackForce;
                DamageType->KnockbackDirection = (Player->GetActorLocation() - OwnerEnemy->GetActorLocation()).GetSafeNormal();



                UGameplayStatics::ApplyPointDamage(
                    Player,
                    Damage,
                    Hit.Location,
                    Hit,
                    OwnerEnemy->GetController(),
                    OwnerEnemy,
                    DamageType->GetClass()
                );

                Player->Server_PlayHitReaction(HitReactionType, KnockbackDir, KnockbackForce);

                AlreadyHitActors.Add(HitActor);
            }
        }
    }
}