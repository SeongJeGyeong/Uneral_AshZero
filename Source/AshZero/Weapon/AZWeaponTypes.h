// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AZStatusTypes.h"
#include "AZWeaponTypes.generated.h"

// 발사 모드
UENUM(BlueprintType)
enum class EFireMode : uint8
{
    Single,     // 단발
    Auto        // 연사
};
// 무기 종류
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Pistol      UMETA(DisplayName = "Pistol"),
    Rifle       UMETA(DisplayName = "Rifle"),
    Sniper      UMETA(DisplayName = "Sniper")

};

/**
 * 무기 특수 효과 타입
 */
UENUM(BlueprintType)
enum class EWeaponEffect : uint8
{
    None                UMETA(DisplayName = "없음"),
    AttackSpeedUp       UMETA(DisplayName = "공격 속도 증가"),
    Bleeding            UMETA(DisplayName = "출혈"),
    Execute             UMETA(DisplayName = "처형"),
    CriticalShot        UMETA(DisplayName = "크리티컬"),
    LifeSteal           UMETA(DisplayName = "흡혈"),
    StackDamage         UMETA(DisplayName = "스택 데미지"),
    MultiShot           UMETA(DisplayName = "멀티샷"),
    Piercing            UMETA(DisplayName = "관통"),
    Explosion           UMETA(DisplayName = "폭발")
};

/**
 * 특수 효과 데이터
 *
 * ===== 효과별 Value 사용법 =====
 *
 * CriticalShot (크리티컬)
 *   - Value1: 확률 (0.1 = 10%)
 *   - Value2: 데미지 배율 (2.0 = 2배)
 *
 * Bleeding (출혈)
 *   - Value1: 틱당 데미지
 *   - Value2: 지속시간 (초)
 *
 * Execute (처형)
 *   - Value1: 체력 임계값 (0.1 = 10% 이하일 때 즉사)
 *
 * LifeSteal (흡혈)
 *   - Value1: 회복 비율 (0.05 = 최대체력의 5%)
 *
 * StackDamage (스택 데미지)
 *   - Value1: 필요 스택 수
 *   - Value2: 데미지 배율
 *
 * MultiShot (멀티샷)
 *   - Value1: 추가 발사 수
 *   - Value2: 스프레드 각도
 *
 * Piercing (관통)
 *   - Value1: 관통 횟수
 *   - Value2: 관통 시 데미지 감소율
 *
 * Explosion (폭발)
 *   - Value1: 폭발 반경
 *   - Value2: 폭발 데미지 비율
 */
USTRUCT(BlueprintType)
struct FWeaponEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EWeaponEffect EffectType = EWeaponEffect::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (ClampMin = "0"))
    float Value1 = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (ClampMin = "0"))
    float Value2 = 0.0f;

    FWeaponEffectData() = default;

    FWeaponEffectData(EWeaponEffect InType, float InValue1 = 0.0f, float InValue2 = 0.0f)
        : EffectType(InType), Value1(InValue1), Value2(InValue2) {
    }

    FAZStatusEffectPacket ToBleedingPacket() const
    {
        FAZStatusEffectPacket Packet;
        Packet.StatusType = EAZStatusType::Bleeding;
        Packet.ValuePerTick = Value1;       // 틱당 데미지
        Packet.Duration = Value2;           // 지속시간
        Packet.TickInterval = 0.5f;         // 0.5초마다 틱
        return Packet;
    }
};

/**
 * 무기 단계별 데이터
 * BP에서 배열로 추가하여 각 단계 설정
 */
USTRUCT(BlueprintType)
struct FWeaponLevelData
{
    GENERATED_BODY()

    // ===== 기본 스탯 =====

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Damage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxAmmo = 30;

    /** 초당 발사 횟수 (FireRate 2.0 = 초당 2발) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.1"))
    float FireRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ReloadTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxRange = 10000.0f;

    // ===== 특수 효과 (여러 개 추가 가능) =====

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TArray<FWeaponEffectData> Effects;

    // ===== 헬퍼 함수 =====

    /** 해당 효과가 있는지 확인 */
    bool HasEffect(EWeaponEffect EffectType) const
    {
        for (const FWeaponEffectData& Effect : Effects)
        {
            if (Effect.EffectType == EffectType)
            {
                return true;
            }
        }
        return false;
    }

    /** 효과 데이터 가져오기 */
    FWeaponEffectData GetEffectData(EWeaponEffect EffectType) const
    {
        for (const FWeaponEffectData& Effect : Effects)
        {
            if (Effect.EffectType == EffectType)
            {
                return Effect;
            }
        }
        return FWeaponEffectData();
    }

    /** FireTime 계산 (FireRate의 역수) */
    float GetFireTime() const
    {
        return (FireRate > 0.0f) ? (1.0f / FireRate) : 1.0f;
    }
};

/**
 * 적 스택 정보 (StackDamage용)
 */
USTRUCT()
struct FEnemyStackInfo
{
    GENERATED_BODY()

    UPROPERTY()
    int32 CurrentStacks = 0;

    UPROPERTY()
    float LastHitTime = 0.0f;
};