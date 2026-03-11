// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/AZInteractable.h"
#include "AZInteractComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZInteractComponent : public UActorComponent, public IAZInteractable
{
	GENERATED_BODY()

public:	
	UAZInteractComponent();

protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION()
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

};
