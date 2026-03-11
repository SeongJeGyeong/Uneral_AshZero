// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AZLobbyGameState.generated.h"

class AAZPlayerState;

DECLARE_MULTICAST_DELEGATE(FOnPartyUpdated);

UCLASS()
class ASHZERO_API AAZLobbyGameState : public AGameState
{
	GENERATED_BODY()

public:
	FOnPartyUpdated OnPartyUpdated;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	void BindPlayerState(AAZPlayerState* AZPlayerState);

	//UFUNCTION(NetMulticast, Reliable)
	//void PartyUpdated_Multicast();

	UFUNCTION(Server, Reliable)
	void PartyUpdated_Server();

protected:
	virtual void BeginPlay() override;
	virtual void HandleMatchHasStarted() override;
};
