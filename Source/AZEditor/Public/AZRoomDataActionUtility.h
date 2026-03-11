// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "AshZero/DataAsset/AZRoomDataAsset.h"
#include "AZRoomDataActionUtility.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AZEDITOR_API UAZRoomDataActionUtility : public UAssetActionUtility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(CallInEditor, Category = "AZ Room Tools")
	void UpdateRoomDataFromLevel();

	bool UpdateRoomData(FRoomData& RoomData, const FString& LevelPackageName);
};
