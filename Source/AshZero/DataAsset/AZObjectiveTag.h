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
	// 핵심: Categories 메타데이터를 사용하여 Quest.Target 하위 태그만 드롭다운에 노출합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TagNameMap")
	TMap<FGameplayTag, FText> ObjectiveNameMap;
};
