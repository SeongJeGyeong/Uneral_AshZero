// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AZLobbyGameState.h"
#include "System/AZSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Util/AZDefine.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "System/AZPlayerState.h"

void AAZLobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	OnPartyUpdated.Broadcast();

	if (AAZPlayerState* AZPS = Cast<AAZPlayerState>(PlayerState))
	{
		BindPlayerState(AZPS);
	}
}

void AAZLobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	OnPartyUpdated.Broadcast();
}

void AAZLobbyGameState::BindPlayerState(AAZPlayerState* AZPlayerState)
{
	//AZPlayerState->OnReadyStateChanged.AddLambda([this]()
	//	{
	//		OnPartyUpdated.Broadcast();
	//	});

	if (AZPlayerState)
	{
		AZPlayerState->OnPlayerStateChanged.AddLambda([this]()
			{
				OnPartyUpdated.Broadcast();
			});
	}
}

//void AAZLobbyGameState::PartyUpdated_Multicast_Implementation()
//{
//	OnPartyUpdated.Broadcast();
//}

void AAZLobbyGameState::PartyUpdated_Server_Implementation()
{
	OnPartyUpdated.Broadcast();
}

void AAZLobbyGameState::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UAZSessionSubsystem>();
	if (SessionSubsystem == nullptr)
		return;
	SessionSubsystem->SetCurrentGameState(EGameState::Lobby);

}

void AAZLobbyGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	//UGameInstance* GI = GetWorld()->GetGameInstance();
	//if (GI == nullptr)
	//	return;
	//UAZSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UAZSessionSubsystem>();
	//if (SessionSubsystem == nullptr)
	//	return;
	//SessionSubsystem->SetCurrentGameState(EGameState::Lobby);
}
