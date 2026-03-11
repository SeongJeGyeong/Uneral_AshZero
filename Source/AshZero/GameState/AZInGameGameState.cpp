// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AZInGameGameState.h"
#include "System/AZSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Util/AZDefine.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "Net/UnrealNetwork.h"

void AAZInGameGameState::DecreaseAlivePlayers()
{
	if (!HasAuthority())
		return;

	AlivePlayers = FMath::Max(AlivePlayers - 1, 0);
}

void AAZInGameGameState::BeginPlay()
{
	Super::BeginPlay();
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UAZSessionSubsystem>();
	if (SessionSubsystem == nullptr)
		return;
	SessionSubsystem->SetCurrentGameState(EGameState::InGame);

	if (HasAuthority())
	{
		AlivePlayers = PlayerArray.Num();
	}
}

void AAZInGameGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void AAZInGameGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAZInGameGameState, AlivePlayers);
}
