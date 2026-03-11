// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AZInGameGameState.generated.h"

/**
 * 
 */
UCLASS()
class ASHZERO_API AAZInGameGameState : public AGameState
{
	GENERATED_BODY()

public:
	int32 GetAlivePlayers() const { return AlivePlayers; };
	void DecreaseAlivePlayers();

protected:
	virtual void BeginPlay() override;
	virtual void HandleMatchHasStarted() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	int32 AlivePlayers = 0;
};
