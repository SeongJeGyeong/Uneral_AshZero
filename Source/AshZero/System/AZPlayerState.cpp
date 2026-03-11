// Fill out your copyright notice in the Description page of Project Settings.


#include "System/AZPlayerState.h"
#include "Net/UnrealNetwork.h"

AAZPlayerState::AAZPlayerState()
{
	bReplicates = true;
}

void AAZPlayerState::SetReady(bool bReady)
{
	if (HasAuthority())
	{
		bIsReady = bReady;
		//OnReadyStateChanged.Broadcast();
	}
}

void AAZPlayerState::OnRep_ReadyState()
{
	//OnReadyStateChanged.Broadcast();
}

void AAZPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	OnPlayerStateChanged.Broadcast();
}

void AAZPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAZPlayerState, bIsReady);
}
