// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "AZItemDataTableRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZItemDataTableRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	FName ItemName;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	//FString Description;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	//int32 Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 MaxStack = 1;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	//float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	EEquipmentSlot EquipmentType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Rows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Columns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	TSoftObjectPtr<UMaterialInstance> MaterialAsset;
};
