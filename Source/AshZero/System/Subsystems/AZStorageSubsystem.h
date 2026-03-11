// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Util/AZDefine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AZStorageSubsystem.generated.h"

class APlayerController;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZStorageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SavePlayerData(APlayerController* PC);
	UFUNCTION(BlueprintCallable)
	void LoadPlayerData(APlayerController* PC);
	UFUNCTION(BlueprintCallable)
	void ClearAllItems(APlayerController* PC);

private:
	UPROPERTY()
	FAZPlayerSaveData SavedPlayerData;
	
};
