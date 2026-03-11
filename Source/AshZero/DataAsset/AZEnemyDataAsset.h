// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Util/AZDefine.h"
#include "AZEnemyDataAsset.generated.h"

class AAZEnemyBase;
class AAZBossBase;

UCLASS()
class ASHZERO_API UAZEnemyDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	TArray<TSubclassOf<AAZEnemyBase>> EnemyList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Data")
	TMap<EBossType, TSubclassOf<AAZBossBase>> BossList;
};
