// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Util/AZDefine.h"
#include "AZUISoundDataAsset.generated.h"

class USoundBase;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZUISoundDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TMap<EUISFXType, TObjectPtr<USoundBase>> UISFXList;

	TObjectPtr<USoundBase> GetUISFX(EUISFXType SelectSFXType);
};
