// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/AZQuestObjective.h"
#include "AZQuestObjectiveComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZQuestObjectiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZQuestObjectiveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, meta = (AloowPrivateAccess = "true"))
	FGameplayTag ObjectiveTag;

	UFUNCTION()
	void OnDeathOwner(FVector DeathLocation, AActor* Killer);

	UFUNCTION()
	void OnInteractNPC(AActor* NPC);

	void SendObjectiveProgress(const UObject* WorldContextObject, const FGameplayTag& Tag, EQuestObjectiveType Type, int32 Count);

	UPROPERTY()
	TObjectPtr<AActor> Owner;
};
