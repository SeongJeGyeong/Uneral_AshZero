// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Util/AZDefine.h"
#include "AZSequenceDataAsset.generated.h"

class ULevelSequence;

UCLASS()
class ASHZERO_API UAZSequenceDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EBossType, TObjectPtr<ULevelSequence>> SequenceMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ULevelSequence> PlayerSequence;
};