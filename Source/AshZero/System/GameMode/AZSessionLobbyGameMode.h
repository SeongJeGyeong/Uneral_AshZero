// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Util/AZDefine.h"
#include "AZSessionLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ASHZERO_API AAZSessionLobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGM")
	EBGMType DefaultBGMType;

public:
	AAZSessionLobbyGameMode();

	virtual void Logout(AController* Exiting) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

protected:
	virtual void BeginPlay() override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;

};
