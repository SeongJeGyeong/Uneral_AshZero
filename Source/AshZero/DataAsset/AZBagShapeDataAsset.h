// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Util/AZDefine.h"
#include "AZBagShapeDataAsset.generated.h"


/**
 * 
 */
UCLASS()
class ASHZERO_API UAZBagShapeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (TitleProperty = "BagName"))
	TArray<FBagDefinition> BagList;

public:

};
