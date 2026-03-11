// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Util/AZDefine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AdvancedSessions/Classes/AdvancedSessionsLibrary.h"
#include "AZSessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, EGameState, NewState);

UCLASS()
class ASHZERO_API UAZSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	bool IsPlayerInSession();

	UFUNCTION(BlueprintCallable)
	void StartGame(const FString& inURL);

	UFUNCTION(BlueprintCallable)
	void ExitSession();

	UFUNCTION(BlueprintCallable)
	TArray<FSessionPropertyKeyPair> CreateExtraSettings();

	UFUNCTION(BlueprintCallable)
	void FilterSessionSearchResults(TArray<FBlueprintSessionResult>& SearchResults);

	UFUNCTION(BlueprintCallable)
	bool IsValidSession(const FBlueprintSessionResult& SearchResult);

	bool UpdateAdvertiseSetting();

	TArray<FSessionPropertyKeyPair> GetSessionExtraSettings(FOnlineSessionSearchResult SearchResult);
	FString GetSessionPropertyString(const TArray<FSessionPropertyKeyPair>& ExtraSettings, FName SettingName);
	int32 GetCurrentPlayers(const FOnlineSessionSearchResult& Result);
	int32 GetMaxPlayers(const FOnlineSessionSearchResult& Result);
	int32 GetPingInMs(const FOnlineSessionSearchResult& Result);
	bool GetHasAuthority();

	void TryJoinSession(const FOnlineSessionSearchResult& Result);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AZ|Network")
	FString SteamId;

	UPROPERTY(BlueprintReadWrite, Category = "AZ|GameState")
	EGameState CurrentGameState = EGameState::Lobby;

	UPROPERTY(BlueprintAssignable)
	FOnGameStateChanged OnGameStateChanged;

	UFUNCTION(BlueprintCallable, Category = "AZ|GameState")
	void SetCurrentGameState(EGameState NewState);

	UPROPERTY()
	int32 RandomSeed = 0;
	int64 CachedGlobalTime = INDEX_NONE;

	void UpdateRandomSeed();

	const int32 SeedInterval = 1800;

private:
	//IOnlineSessionPtr SessionInterface;

	FName CurrentSessionName = NAME_GameSession;
	bool bAllow = true;

	FDelegateHandle CreateSessionCompleteHandle;

	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION()
	void OnSessionUpdateComplete(FName SessionName, bool bWasSuccessful);

	void MakeConnectedAllPlayerState();
};
