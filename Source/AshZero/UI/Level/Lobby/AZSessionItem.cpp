// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Level/Lobby/AZSessionItem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "System/AZSessionSubsystem.h"

void UAZSessionItem::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAZSessionItem::UpdateSessionInfo(const FBlueprintSessionResult& SearchResult)
{
	UAZSessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UAZSessionSubsystem>();
	if (!SessionSubsystem) return;
	if (!SearchResult.OnlineResult.IsValid()) return;

	SessionResult = SearchResult;
	TArray<FSessionPropertyKeyPair> ExtraSettings = SessionSubsystem->GetSessionExtraSettings(SearchResult.OnlineResult);
	FString SettingValue = SessionSubsystem->GetSessionPropertyString(ExtraSettings, FName("RoomName"));

	if (!SettingValue.IsEmpty())
		SessionName->SetText(FText::FromString(SettingValue));

	int32 CurPlayer = SearchResult.OnlineResult.Session.SessionSettings.NumPublicConnections
					- SearchResult.OnlineResult.Session.NumOpenPublicConnections;
	int32 MaxPlayer = SearchResult.OnlineResult.Session.SessionSettings.NumPublicConnections;

	FText Players = FText::FromString(FString::FromInt(CurPlayer) + "/" + FString::FromInt(MaxPlayer));
	NumOfPlayers->SetText(Players);

	FString PingMs = FString::FromInt(SearchResult.OnlineResult.PingInMs);
	Ping->SetText(FText::FromString(PingMs+"ms"));
}