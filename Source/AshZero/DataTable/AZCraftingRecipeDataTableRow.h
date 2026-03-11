// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "AZCraftingRecipeDataTableRow.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZCraftingRecipeDataTableRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 JobID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 ResultItemID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Mat_1_ID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Mat_1_Qty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Mat_2_ID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Mat_2_Qty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Mat_3_ID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Mat_3_Qty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Cost;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float Craft_Time_sec;
};
