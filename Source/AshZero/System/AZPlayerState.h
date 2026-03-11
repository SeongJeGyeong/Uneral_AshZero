// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AZPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnReadyStateChanged);
DECLARE_MULTICAST_DELEGATE(FOnPlayerStateChanged);

UCLASS()
class ASHZERO_API AAZPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AAZPlayerState();

	UPROPERTY(Replicated)
	bool bIsReady = false;

	void SetReady(bool bReady);

	FOnPlayerStateChanged OnPlayerStateChanged;
protected:
	UFUNCTION()
	void OnRep_ReadyState();
	virtual void OnRep_PlayerName() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


};
