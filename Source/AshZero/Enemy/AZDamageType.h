// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "Util/AZDefine.h"
#include "AZDamageType.generated.h"

// AZDamageType.h
UCLASS()
class ASHZERO_API UAZDamageType : public UDamageType
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    EHitReactionType HitReactionType = EHitReactionType::LightHit;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float KnockbackForce = 800.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FVector KnockbackDirection = FVector::ZeroVector;
};