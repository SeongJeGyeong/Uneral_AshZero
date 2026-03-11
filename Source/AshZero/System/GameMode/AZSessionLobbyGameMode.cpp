// Fill out your copyright notice in the Description page of Project Settings.


#include "AZSessionLobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"

AAZSessionLobbyGameMode::AAZSessionLobbyGameMode()
{
	bUseSeamlessTravel = true;
}

void AAZSessionLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void AAZSessionLobbyGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
}

void AAZSessionLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	//if (UAZSoundManagerSubsystem* SoundSystem = GetGameInstance()->GetSubsystem<UAZSoundManagerSubsystem>())
	//{
	//	SoundSystem->PlayBGM(DefaultBGMType);
	//}
}

FString AAZSessionLobbyGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	if (NewPlayerController->IsLocalController())
	{
		//ChangeName(NewPlayerController, GetGameInstance<UIHGameInstance>()->PlayerName, false);
	}
	else
	{
		// Init player's name
		FString InName = UGameplayStatics::ParseOption(Options, TEXT("PlayerName")).Left(20);
		if (InName.IsEmpty() == false)
		{
			ChangeName(NewPlayerController, InName, false);
		}
	}

	return ErrorMessage;
}
