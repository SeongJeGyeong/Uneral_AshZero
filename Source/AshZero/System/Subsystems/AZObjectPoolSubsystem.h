// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Util/AZDefine.h"
#include "Enemy/AZEnemyBase.h"
#include "Enemy/AZBossGigantia.h"
#include "Enemy/AZBossArgus.h"
#include "Enemy/AZBossBase.h"
#include "AZObjectPoolSubsystem.generated.h"

//class AAZEnemyBase;
//class AAZBossGigantia;
//class AAZBossArgus;
//class AAZBossBase;

UCLASS()
class ASHZERO_API UAZObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UAZObjectPoolSubsystem();

	AAZBossBase* SpawnBoss(EBossType Type, const FTransform& SpawnTransform);

	UPROPERTY()
	TArray<TSubclassOf<AAZEnemyBase>> EnemyClassList;

	UPROPERTY()
	TSubclassOf<AAZBossGigantia> SpineQueenClass;

	UPROPERTY()
	TSubclassOf<AAZBossArgus> ArgusClass;

	UPROPERTY()
	TSubclassOf<AAZBossBase> ZephyrosClass;

private:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
