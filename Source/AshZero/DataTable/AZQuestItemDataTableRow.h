// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Util/AZDefine.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "AZQuestItemDataTableRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ASHZERO_API FAZQuestItemDataTableRow : public FAZBaseItemDataTable
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float QuestID_Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	float QuestID_Unlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	bool IsDisappearOnUse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	bool IsCraftingMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	bool IsDiscardable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Data")
	bool IsLostOnDeath;



};
