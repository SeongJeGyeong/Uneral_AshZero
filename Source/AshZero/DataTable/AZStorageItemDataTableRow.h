// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "AZStorageItemDataTableRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZStorageItemDataTableRow : public FAZBaseItemDataTable
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Purchase_Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 BaseValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float MoveSpeed_Penalty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float TurnSpeed_Penalty;
};
