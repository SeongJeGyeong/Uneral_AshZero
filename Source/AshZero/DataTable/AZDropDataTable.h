// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "AZDropDataTable.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZDropDataTable: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float DropChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 MinCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 MaxCount;
};
