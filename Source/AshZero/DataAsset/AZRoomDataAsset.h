// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Levels/AZRandomMapGenerator.h"
#include "AZRoomDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ASHZERO_API UAZRoomDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Data")
	TArray<FSpawnLevelData> RoomDataList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Room Data")
	FSpawnLevelData BossRoomData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Room Data")
	FSpawnLevelData SpecialRoomData;
};
