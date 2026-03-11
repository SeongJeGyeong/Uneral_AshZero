// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AZBossBase.h"
#include "AZBossArgus.generated.h"

/**
 * 
 */
UCLASS()
class ASHZERO_API AAZBossArgus : public AAZBossBase
{
	GENERATED_BODY()

public:
	AAZBossArgus();

protected:
	virtual void BeginPlay() override;

	// ===== 전용 설정 =====
public:


};