// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AZBossBase.h"
#include "AZBossGigantia.generated.h"

/**
 * 중간보스 기간티아
 *
 * 페이즈 1 (100~40%)
 * - 이중 강타, 엇박 이중 강타, 돌진 공격, 빠른 돌진 공격
 *
 * 페이즈 2 (40~0%): 원거리 + 독장판 추가
 * - 이중 강타, 엇박 이중 강타, 돌진 공격, 빠른 돌진 공격
 *   잠행 공격, 독 장판 생성, 3갈래 촉수 발사, 촉수 레이저
 */
UCLASS()
class ASHZERO_API AAZBossGigantia : public AAZBossBase
{
	GENERATED_BODY()

public:
	AAZBossGigantia();

protected:
	virtual void BeginPlay() override;

};