// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "AZSuppliesItemDataTableRow.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZSuppliesItemDataTableRow : public FAZBaseItemDataTable
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "13"))
	int32 Purchase_Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "12"))
	int32 BaseValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	EEquipmentSlot EquipmentSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "14"))
	int32 MaxUses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "15"))
	float UseTime_Sec;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "16"))
	float HealAmount_HP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "17"))
	EItemBuffType Buff_Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "18"))
	float Buff_Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "19"))
	float Buff_Duration_Sec;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "29"))
	int32 Hunger_Restore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "21"))
	int32 Thirst_Restore;

};
