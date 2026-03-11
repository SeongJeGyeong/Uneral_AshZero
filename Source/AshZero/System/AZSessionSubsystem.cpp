// Fill out your copyright notice in the Description page of Project Settings.


#include "System/AZSessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameFramework/PlayerState.h"
#include "AdvancedSessions/Classes/AdvancedSessionsLibrary.h"
#include "GameFramework/GameMode.h"
#include "Player/AZPlayerController.h"
#include "Components/AZPlayerInventoryComponent.h"
#include "GameState/AZLobbyGameState.h"
#include "System/Subsystems/AZStorageSubsystem.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/AZHealthComponent.h"

void UAZSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem) return;

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UAZSessionSubsystem::OnCreateSessionComplete));
		SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &UAZSessionSubsystem::OnSessionUpdateComplete));
	}
}

bool UAZSessionSubsystem::IsPlayerInSession()
{
	UWorld* const World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController null"));
		return false;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem) return false;

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface) return false;

	IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
	if (!OnlineSubsystemInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystemInterface null"));
		return false;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (Session != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session true"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session false"));
	}
	
	return (Session != nullptr);
}

void UAZSessionSubsystem::StartGame(const FString& inURL)
{
	UWorld* world = GetWorld();
	if (!world) return;

	//MakeConnectedAllPlayerState();

	/*AIHLobbyState* lobbyState = Cast<AIHLobbyState>(world->GetGameState());
	if (lobbyState)
	{
		lobbyState->ShowLoadingScreen_Multicast();
	}*/
	CurrentGameState = EGameState::InGame;
	for (FConstPlayerControllerIterator It = world->GetPlayerControllerIterator(); It ; ++It)
	{
		AAZPlayerController* PC = Cast<AAZPlayerController>(It->Get());
		if (PC)
		{
			PC->PreTravelCleanUp_Client();
			//PC->InventoryComp->ResetInventoryData_Server();
		}
	}
	FTimerHandle TimerHandle;
	world->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, inURL]()
		{
			UWorld* world = GetWorld();
			if (!world) return;
			if (world)
			{
				world->ServerTravel(inURL);
			}
		}),1.0f, false);

}

void UAZSessionSubsystem::ExitSession()
{
	UWorld* world = GetWorld();
	if (!world) return;

	UGameInstance* GameInstance = GetGameInstance();
	UAZStorageSubsystem* StorageSubsystem = nullptr;
	if (GameInstance)
	{
		StorageSubsystem = GameInstance->GetSubsystem<UAZStorageSubsystem>();
	}

	AGameMode* GameMode = world->GetAuthGameMode<AGameMode>();

	// 게임 모드가 있으면 서버
	if (GameMode)
	{
		APlayerController* HostPC = world->GetFirstPlayerController();
		if (StorageSubsystem && HostPC)
		{
			AAZPlayerCharacter* AZChar = Cast<AAZPlayerCharacter>(HostPC->GetPawn());
			if (AZChar && AZChar->HealthComp)
			{
				if (AZChar->HealthComp->Hp <= 0)
				{
					StorageSubsystem->ClearAllItems(HostPC);
				}
				else
				{
					StorageSubsystem->SavePlayerData(HostPC);
				}
			}
		}

		GameMode->ReturnToMainMenuHost();
		AAZPlayerController* Controller = NULL;
		FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator();

		// 세션을 파괴하고 모든 인원을 내보냄
		for (; Iterator; ++Iterator)
		{
			Controller = Cast<AAZPlayerController>(Iterator->Get());
			if (Controller && !Controller->IsLocalPlayerController() && Controller->IsPrimaryPlayer())
			{
				Controller->ReturnToMenu_Client();
				Controller->ClientReturnToMainMenuWithTextReason(FText::GetEmpty());
			}
		}
	}
	// 클라
	else
	{
		if (AAZLobbyGameState* GS = GetWorld()->GetGameState<AAZLobbyGameState>())
		{
			GS->PartyUpdated_Server();
		}

		APlayerController* LocalPC = world->GetFirstPlayerController();
		if (StorageSubsystem && LocalPC)
		{
			AAZPlayerCharacter* AZChar = Cast<AAZPlayerCharacter>(LocalPC->GetPawn());

			bool bIsSpectating = LocalPC->IsInState(NAME_Spectating);
			bool bIsHealthZero = (AZChar && AZChar->HealthComp && AZChar->HealthComp->Hp <= 0);

			if (bIsSpectating || bIsHealthZero)
			{
				StorageSubsystem->ClearAllItems(LocalPC);
			}
			else
			{
				if (AZChar)
				{
					StorageSubsystem->SavePlayerData(LocalPC);
				}
			}
		}

		// 자기 자신의 세션만 파괴하고 나감
		APlayerController* playerController = world->GetFirstPlayerController();
		if (playerController)
		{
			playerController->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("ExitButton")));
		}
	}
}

TArray<FSessionPropertyKeyPair> UAZSessionSubsystem::CreateExtraSettings()
{
	TArray<FSessionPropertyKeyPair> ExtraSettings;

	FSessionPropertyKeyPair GameName;
	GameName.Key = FName("GameName");
	GameName.Data.SetValue(TEXT("AshZero"));
	ExtraSettings.Add(GameName);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	FSessionPropertyKeyPair HostName;
	HostName.Key = FName("HostName");
	HostName.Data.SetValue(PlayerController->GetName());
	ExtraSettings.Add(HostName);

	FSessionPropertyKeyPair AllowAdvertise;
	AllowAdvertise.Key = FName("AllowAdvertise");
	AllowAdvertise.Data.SetValue(true);
	ExtraSettings.Add(AllowAdvertise);

	return ExtraSettings;
}

void UAZSessionSubsystem::FilterSessionSearchResults(TArray<FBlueprintSessionResult>& SearchResult)
{
	for (auto data : SearchResult)
	{
		TArray<FSessionPropertyKeyPair> ExtraSettings = GetSessionExtraSettings(data.OnlineResult);
		FString SettingValue = GetSessionPropertyString(ExtraSettings, FName("HostName"));
	}
}

bool UAZSessionSubsystem::IsValidSession(const FBlueprintSessionResult& SearchResult)
{
	const FOnlineSessionSearchResult& SessionResult = SearchResult.OnlineResult;
	const int32 BuildUniqueId = SessionResult.Session.SessionSettings.BuildUniqueId;
	const int32 Ping = SessionResult.PingInMs;
	const bool bValidInfo = SessionResult.IsValid();

	const bool IsInvalidSession = (BuildUniqueId == 0 && Ping == 9999);

	if (IsInvalidSession || !bValidInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Session"));
		return false;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem) return false;

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return false;

	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController(GetWorld());
	if (!PlayerController) return false;

	IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
	if (!OnlineSubsystemInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystemInterface null"));
		return false;
	}

	return true;
}

bool UAZSessionSubsystem::UpdateAdvertiseSetting()
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem) return false;

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

	FOnlineSessionSettings* CurrentSettings = SessionInterface->GetSessionSettings(NAME_GameSession);
	TMap<FName, FOnlineSessionSetting> Backup = CurrentSettings->Settings;

	//bool bAllow;
	//CurrentSettings->Get("AllowAdvertise", bAllow);
	//bAllow = !bAllow;
	//CurrentSettings->Set("AllowAdvertise", bAllow);

	//FString name;
	//CurrentSettings->Get("GameName", name);
	//CurrentSettings->Set("GameName", TEXT("TestName"));

	for (auto& pair : CurrentSettings->Settings)
	{
		if (pair.Key == "AllowAdvertise")
		{
			bAllow = !bAllow;
			pair.Value.Data = bAllow;
		}
	}

	if (bAllow)
	{
		UE_LOG(LogTemp, Warning, TEXT("AllowAdvertise On"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AllowAdvertise Off"));
	}

	SessionInterface->UpdateSession(NAME_GameSession, *CurrentSettings);

	return bAllow;
}

TArray<FSessionPropertyKeyPair> UAZSessionSubsystem::GetSessionExtraSettings(FOnlineSessionSearchResult SearchResult)
{
	TArray<FSessionPropertyKeyPair> ExtraSettings;

	FSessionPropertyKeyPair NewSetting;
	for (auto& Elem : SearchResult.Session.SessionSettings.Settings)
	{
		NewSetting.Key = Elem.Key;
		NewSetting.Data = Elem.Value.Data;
		ExtraSettings.Add(NewSetting);
	}

	return ExtraSettings;
}

FString UAZSessionSubsystem::GetSessionPropertyString(const TArray<FSessionPropertyKeyPair>& ExtraSettings, FName SettingKey)
{
	FString SettingValue;
	for (FSessionPropertyKeyPair itr : ExtraSettings)
	{
		if (itr.Key == SettingKey)
		{
			if (itr.Data.GetType() == EOnlineKeyValuePairDataType::String)
			{
				itr.Data.GetValue(SettingValue);
				break;
			}
		}
	}

	return SettingValue;
}

int32 UAZSessionSubsystem::GetCurrentPlayers(const FOnlineSessionSearchResult& Result)
{
	return Result.Session.SessionSettings.NumPublicConnections - Result.Session.NumOpenPublicConnections;
}

int32 UAZSessionSubsystem::GetMaxPlayers(const FOnlineSessionSearchResult& Result)
{
	return Result.Session.SessionSettings.NumPublicConnections;
}

int32 UAZSessionSubsystem::GetPingInMs(const FOnlineSessionSearchResult& Result)
{
	return Result.PingInMs;
}

bool UAZSessionSubsystem::GetHasAuthority()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	return (PlayerController && PlayerController->HasAuthority());
}

void UAZSessionSubsystem::TryJoinSession(const FOnlineSessionSearchResult& Result)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (SessionInterface.IsValid())
	{
		SessionInterface->JoinSession(0, NAME_GameSession, Result);
	}
}

void UAZSessionSubsystem::SetCurrentGameState(EGameState NewState)
{
	CurrentGameState = NewState;

	if (OnGameStateChanged.IsBound())
	{
		OnGameStateChanged.Broadcast(CurrentGameState);
	}

	UpdateRandomSeed();
}

void UAZSessionSubsystem::UpdateRandomSeed()
{
	if (!GetWorld() || !GetHasAuthority()) return;

	const int64 CurrentTime = FDateTime::UtcNow().ToUnixTimestamp() / SeedInterval;
	if (CachedGlobalTime == CurrentTime) return;

	CachedGlobalTime = CurrentTime;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FUniqueNetIdPtr UniqueNetId = PlayerController->PlayerState->GetUniqueId().GetUniqueNetId();

	RandomSeed = HashCombine(GetTypeHash(CachedGlobalTime), GetTypeHash(UniqueNetId->ToString()));
}

void UAZSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("sessionCreated"));
	CurrentSessionName = SessionName;
}

void UAZSessionSubsystem::OnSessionUpdateComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateSession Complete: %s (%d)"), *SessionName.ToString(), bWasSuccessful);

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateSession failed!"));
	}
}

void UAZSessionSubsystem::MakeConnectedAllPlayerState()
{
	UWorld* world = GetWorld();
	if (!world) return;

	/*for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		APlayerController* playerController = iter->Get();
		if (playerController && playerController->GetPlayerState<AIHPlayerState>())
		{
			ConnectedPlayerState.Add(playerController->GetPlayerState<AIHPlayerState>()->GetUniqueId(), false);
		}
	}*/
}
