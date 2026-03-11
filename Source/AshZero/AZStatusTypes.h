#pragma once

#include "CoreMinimal.h"
#include "AZStatusTypes.generated.h"

UENUM(BlueprintType)
enum class EAZStatusType : uint8
{
    None,
    Poison          UMETA(DisplayName = "Poison (Debuff)"),
    Bleeding		UMETA(DisplayName = "Bleeding (Debuff)"),
    Fire            UMETA(DisplayName = "Fire Damage (Debuff)"),
    Starvation      UMETA(DisplayName = "Starvation (Debuff)"),
    Dehydration     UMETA(DisplayName = "Dehydration (Debuff)"),

    PowerUp         UMETA(DisplayName = "Attack Power Up (Buff)"),
    MaxHpUp         UMETA(DisplayName = "Max HP Up (Buff)"),
    MaxStaminaUp    UMETA(DisplayName = "Max Stamina Up (Buff)"),
    SpeedBoost      UMETA(DisplayName = "Speed Boost (Buff)"),
	DamageResist    UMETA(DisplayName = "Damage Resistance (Buff)"),

    Invincible      UMETA(DisplayName = "Invincible (Special)")

};

USTRUCT(BlueprintType)
struct FAZStatusEffectPacket
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAZStatusType StatusType = EAZStatusType::None;

    // 지속 시간 (0이면 즉시 적용 혹은 무한)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 5.0f;

    // 수치 변화량 (데미지나 회복량)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ValuePerTick = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TickInterval = 1.0f;

    // 스탯 배율 (1.0 = 100%, 1.2 = 120% 버프, 0.5 = 50% 디버프)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StatMultiplier = 1.0f;

    // UI에 표시될 아이콘 (필요시)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UTexture2D* StatusIcon = nullptr;
};

USTRUCT(BlueprintType)
struct FSurvivalThreshold
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThresholdPercent = 30.0f; // 이 수치 이하로 내려가면

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAZStatusEffectPacket Effect;   // 이 효과 적용
};

// DoT 효과 추적용 구조체
USTRUCT()
struct FActiveDoTEffect
{
    GENERATED_BODY()

    FAZStatusEffectPacket EffectPacket;
    FTimerHandle TickTimerHandle;
    float RemainingDuration = 0.f;
};
