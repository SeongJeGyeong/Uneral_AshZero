// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AZObjectiveTag.generated.h"

/**
 * 
 */
UCLASS()
class ASHZERO_API UAZObjectiveTag : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TagNameMap")
	TMap<FGameplayTag, FText> ObjectiveNameMap;
};
