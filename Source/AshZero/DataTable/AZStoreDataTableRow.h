// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "AZStoreDataTableRow.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZStoreDataTableRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 BaseValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 PurchasePrice;
};
