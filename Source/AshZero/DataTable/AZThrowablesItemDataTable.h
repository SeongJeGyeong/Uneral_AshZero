// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "AZThrowablesItemDataTable.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZThrowablesItemDataTable : public FAZBaseItemDataTable
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Purchase_Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 BaseValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	EEquipmentSlot EquipmentSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float UseTime_Sec;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float Fuse_Time_Sec;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float Explosion_Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	EExplosionType Explosion_Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float Max_Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float Effect_Duration;
};
