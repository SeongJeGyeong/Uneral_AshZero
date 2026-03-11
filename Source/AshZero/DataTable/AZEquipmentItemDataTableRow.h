// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "AZEquipmentItemDataTableRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZEquipmentItemDataTableRow : public FAZBaseItemDataTable
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 BaseValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	EEquipmentSlot EquipmentSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 ArmorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float Gives_Benefit_HP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float Gives_Benefit_Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float Gives_Benefit_Stamina;
};
