// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "AZMaterialItemDataTableRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZMaterialItemDataTableRow : public FAZBaseItemDataTable
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 BaseValue;
};
