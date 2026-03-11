// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "AZSpawnPointComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class ASHZERO_API UAZSpawnPointComponent : public UArrowComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<AActor> OwnerRoom;

};
