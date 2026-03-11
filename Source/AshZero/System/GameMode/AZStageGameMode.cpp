// Fill out your copyright notice in the Description page of Project Settings.


#include "System/GameMode/AZStageGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Levels/AZRandomMapGenerator.h"
#include "System/Player/AZPlayerController.h"
#include "Character/AZPlayerCharacter.h"
#include "GameState/AZInGameGameState.h"
#include "System/AZSessionSubsystem.h"

AAZStageGameMode::AAZStageGameMode()
{
	bUseSeamlessTravel = true;

	bStartPlayersAsSpectators = false;
}

void AAZStageGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AAZStageGameMode::TrySpawnPlayers(const FTransform& SpawnPoint)
{
	for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		APlayerController* PlayerController = it->Get();
		if (PlayerController && PlayerController->GetPawn() == nullptr)
		{
			RestartPlayerAtTransform(PlayerController, SpawnPoint);
		}
	}
}

APawn* AAZStageGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAZRandomMapGenerator::StaticClass(), FoundActors);
	FTransform Spawn = SpawnTransform;

	for (auto Actor : FoundActors)
	{
		AAZRandomMapGenerator* mapGenerator = Cast<AAZRandomMapGenerator>(Actor);
		if (mapGenerator)
		{
			Spawn = mapGenerator->StartPoint;
		}
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(PawnClass, Spawn, SpawnInfo);
	if (!ResultPawn)
	{
		UE_LOG(LogGameMode, Warning, TEXT("SpawnDefaultPawnAtTransform: Couldn't spawn Pawn of type %s at %s"), *GetNameSafe(PawnClass), *Spawn.ToHumanReadableString());
	}
	return ResultPawn;
}

APawn* AAZStageGameMode::GetAliveOtherPlayer(APawn* ExcludePawn)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* Pawn = PC->GetPawn();
		if (Pawn && Pawn != ExcludePawn)
		{
			return Pawn;
		}
	}

	return nullptr;
}

void AAZStageGameMode::HandlePlayerReadyToSpectate(AAZPlayerController* DeadPC)
{
	AAZInGameGameState* InGameState = GetGameState<AAZInGameGameState>();
	if (!InGameState) return;
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UAZSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UAZSessionSubsystem>();
	if (!SessionSubsystem) return;

	if (InGameState->GetAlivePlayers() <= 0)
		SessionSubsystem->ExitSession();

	APawn* TargetPawn = GetAliveOtherPlayer(DeadPC->GetPawn());
	if (TargetPawn)
	{
		DeadPC->TargetPawn = TargetPawn;
		DeadPC->SetSpectatorView_Client(TargetPawn);
	}
	else
	{
		SessionSubsystem->ExitSession();
	}
}

void AAZStageGameMode::TeleportAllPlayersToBossRoom(const FTransform& TeleportTransform)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLocation = TeleportTransform.GetLocation();
	FRotator SpawnRotation = TeleportTransform.Rotator();

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		AAZPlayerCharacter* PlayerChar = Cast<AAZPlayerCharacter>(PC->GetPawn());
		if (PlayerChar)
		{
			PlayerChar->TeleportTo(
				SpawnLocation + FVector(FMath::RandRange(-100, 100), FMath::RandRange(-100, 100), 0),
				SpawnRotation
			);
		}
	}
}

void AAZStageGameMode::HandlePlayerMapLoaded(APlayerController* PC)
{
	LoadCompletePlayerCount++;
	// 현재 접속한 총 플레이어 수 (GetNumPlayers()는 연결된 PC 개수 반환)
	int32 TotalPlayers = GetNumPlayers();

	UE_LOG(LogTemp, Warning, TEXT("Map Loaded: %d / %d"), LoadCompletePlayerCount, TotalPlayers);

	if (LoadCompletePlayerCount >= TotalPlayers)
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			AAZPlayerController* TargetPC = Cast<AAZPlayerController>(Iterator->Get());
			if (TargetPC)
			{
				TargetPC->HideLoadingScreen_Multicast();
			}
		}
	}
}

void AAZStageGameMode::HideLoadingScreenFor(APlayerController* SpecificPC)
{
}

void AAZStageGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	--LoadCompletePlayerCount;
}

bool AAZStageGameMode::ReadyToStartMatch_Implementation()
{
	//return bHandleStartGame;

	UWorld* World = GetWorld();
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		return true;
	}

	return false;
}