// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "AZBaseItemDataTable.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZBaseItemDataTable : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "1"))
	int32 ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "4"))
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "2"))
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "3"))
	FGameplayTag GamePlayTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "5"))
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "6"))
	ERarity Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "9"))
	int32 Rows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "10"))
	int32 Columns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "7"))
	int32 MaxStack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "8"))
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "11"))
	TSoftObjectPtr<UMaterialInstance> MaterialAsset;
};
