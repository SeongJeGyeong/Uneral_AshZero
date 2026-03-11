// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AZNormalEnemy.h"
#include "Enemy/AZEnemyFSM.h"
#include "Components/AZHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Character/AZPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "AshZero.h"

AAZNormalEnemy::AAZNormalEnemy()
{
    // 기본 메시 설정 (IHMinion 참고)
    //static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/Assets/Monster/Frog/Mesh/SK_112111.SK_112111'"));  // 에셋 경로
   /* if (MeshAsset.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);
        GetMesh()->SetRelativeLocation(FVector(0, 0, -88));
        GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
    }*/

    // 기본 스탯 설정
    if (HealthComponent)
    {
        HealthComponent->MaxHp = BaseHealth;
    }

    if (FSMComponent)
    {
        FSMComponent->Damage = BaseDamage;
        FSMComponent->PatrolRadius = 1000.0f;  // 미니언은 좁은 범위 패트롤
    }

    // 아이템 드롭 설정
    ItemDropChance = 0.5f;  // 50% 확률

    bReplicates = true;
}

void AAZNormalEnemy::BeginPlay()
{
    Super::BeginPlay();
}

void AAZNormalEnemy::SpecialAbility()
{
}

