// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AZStageGameMode.generated.h"

class AAZPlayerController;

UCLASS()
class ASHZERO_API AAZStageGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	bool bIsSimulate = false;
	bool bHandleStartGame = false;

	AAZStageGameMode();

	virtual void BeginPlay() override;

	void TrySpawnPlayers(const FTransform& SpawnPoint);

	virtual bool ReadyToStartMatch_Implementation() override;

	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	APawn* GetAliveOtherPlayer(APawn* ExcludePawn);

	void HandlePlayerReadyToSpectate(AAZPlayerController* DeadPC);

	void TeleportAllPlayersToBossRoom(const FTransform& TeleportTransform);
	
	void HandlePlayerMapLoaded(APlayerController* PC);

	void HideLoadingScreenFor(APlayerController* SpecificPC = nullptr);

	UPROPERTY(Transient)
	FTransform StartPoint;

private:
	int32 LoadCompletePlayerCount = 0;

	bool bMatchStarted = false;

	UPROPERTY()
	TSet<APlayerController*> ReadyControllers;

	virtual void Logout(AController* Exiting) override;

};
