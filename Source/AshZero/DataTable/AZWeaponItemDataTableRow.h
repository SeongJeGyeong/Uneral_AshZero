// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "AZWeaponItemDataTableRow.generated.h"

class AAZWeapon;
/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZWeaponItemDataTableRow : public FAZBaseItemDataTable
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "13"))
	int32 Purchase_Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data", meta = (DisplayPriority = "12"))
	int32 BaseValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	EEquipmentSlot EquipmentSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	FString Weapon_Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float Base_Atk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Magazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float AtkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float ReloadSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	TSubclassOf<AAZWeapon> WeaponActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	int32 Level;
};
