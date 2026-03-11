// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Util/AZDefine.h"
#include "AZInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAZInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ASHZERO_API IAZInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	EHighlightColor HighlightColor = EHighlightColor::White;
	FText InteractionText = FText::FromString(TEXT("Loot"));
	FKey InteractionKey = FKey(FName("F"));
	FText ActorName = FText::FromString(TEXT("Box"));

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* InstigatorActor);
};
