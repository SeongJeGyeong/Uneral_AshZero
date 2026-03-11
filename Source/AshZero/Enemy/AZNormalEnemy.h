// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AZEnemyBase.h"
#include "AZNormalEnemy.generated.h"

UCLASS()
class ASHZERO_API AAZNormalEnemy : public AAZEnemyBase
{
	GENERATED_BODY()
	
public:
	AAZNormalEnemy();


protected:
    virtual void BeginPlay() override;

public:
    // 미니언 타입별 설정
    UPROPERTY(EditDefaultsOnly, Category = "AZ|Stats")
    float BaseHealth = 50.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Stats")
    float BaseDamage = 5.0f;

    // 특수 능력 (확장용)
    UFUNCTION(BlueprintCallable, Category = "AZ|Abilities")
    virtual void SpecialAbility();
};
