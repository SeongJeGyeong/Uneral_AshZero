// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AZSequencePivot.generated.h"

UCLASS()
class ASHZERO_API AAZSequencePivot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZSequencePivot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
