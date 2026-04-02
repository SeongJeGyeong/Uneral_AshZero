// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Subsystems/AZSceneSubsystem.h"
#include "System/Settings/AZDeveloperSettings.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/AZPlayerMoveComponent.h"
#include "Components/Image.h"
#include "UI/Scene/AZLoadingScreen.h"
#include "DataTable/AZTipsTableRow.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"
#include "DataAsset/AZSequenceDataAsset.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "System/Player/AZPlayerController.h"
#include "Components/AZHealthComponent.h"

void UAZSceneSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &UAZSceneSubsystem::BeginLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UAZSceneSubsystem::EndLoadMap);
	FWorldDelegates::OnSeamlessTravelStart.AddUObject(this, &UAZSceneSubsystem::BeginSeamlessTravel);

	const UAZDeveloperSettings* Settings = GetDefault<UAZDeveloperSettings>();
	if (!Settings || !Settings->LoadingScreenClass) return;
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	SequenceMap = Settings->SequenceData.LoadSynchronous();

	UDataTable* TipTable = Settings->TipsDataTable.LoadSynchronous();
	if (!TipTable || TipTable->GetRowStruct() != FAZTipsTableRow::StaticStruct())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load TipsDataTable"));
		return;
	}

	TArray<FAZTipsTableRow*> Rows;
	TipTable->GetAllRows<FAZTipsTableRow>(TEXT("TipInit"), Rows);

	Tips.Empty();
	for (const FAZTipsTableRow* Row : Rows)
		Tips.Add(Row->Tip);
}

void UAZSceneSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PreLoadMapWithContext.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	if (LoadingWidget.IsValid())
		LoadingWidget.Reset();

	Super::Deinitialize();
}

bool UAZSceneSubsystem::IsPIE(const UWorld* World)
{
	return GIsEditor && World && World->WorldType == EWorldType::PIE;
}

ULevelSequence* UAZSceneSubsystem::GetBossSequence(EBossType Type)
{
	return SequenceMap ? SequenceMap->SequenceMap.FindRef(Type) : nullptr;
}

void UAZSceneSubsystem::PlayPlayerSequence()
{
	ALevelSequenceActor* OutActor;
	SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(),
		SequenceMap->PlayerSequence,
		FMovieSceneSequencePlaybackSettings(),
		OutActor
	);

	if (SequencePlayer)
	{
		SequencePlayer->Play();
		SequencePlayer->OnFinished.AddDynamic(this, &UAZSceneSubsystem::FinishPlayerSequence);
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (AAZPlayerCharacter* Character = Cast<AAZPlayerCharacter>(PC->GetPawn()))
		{
			Character->MoveComp->ResetMoveInput();
			Character->GetCharacterMovement()->StopMovementImmediately();
			Character->DisableInput(NULL);
			if (Character->HealthComp)
				Character->HealthComp->SetHealth(Character->HealthComp->GetMaxHp());
		}

		if (AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC))
			AZPC->SetHUDVisibility(ESlateVisibility::Collapsed);
	}
}

void UAZSceneSubsystem::FinishPlayerSequence()
{
	SequencePlayer->OnFinished.RemoveDynamic(this, &UAZSceneSubsystem::FinishPlayerSequence);
	SequencePlayer = nullptr;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (APawn* Pawn = PC->GetPawn())
			Pawn->EnableInput(NULL);

		if (AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC))
			AZPC->SetHUDVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UAZSceneSubsystem::BeginLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
	if (WorldContext.OwningGameInstance != GetGameInstance()) return;
	if (TEXT("/Game/Maps/LobbyLevel/Lobbylevel_Hub") == MapName) return;

	UE_LOG(LogTemp, Warning, TEXT("MapName: %s"), *MapName);
	UE_LOG(LogTemp, Warning, TEXT("BeginLoadMap"));

	ShowLoadingScreen();
}

void UAZSceneSubsystem::EndLoadMap(UWorld* InLoadedWorld)
{
	if (InLoadedWorld == nullptr || InLoadedWorld->GetGameInstance() != GetGameInstance()) return;

	UE_LOG(LogTemp, Warning, TEXT("MapName: %s"), *InLoadedWorld->GetFName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("EndLoadMap"));

	if (!InLoadedWorld->GetFName().IsEqual(TEXT("FieldLevel")))
		HideLoadingScreen();
}

void UAZSceneSubsystem::BeginSeamlessTravel(UWorld* World, const FString& MapName)
{
	if (World == nullptr || World->GetGameInstance() != GetGameInstance()) return;

	ShowLoadingScreen();
}

void UAZSceneSubsystem::ShowLoadingScreen()
{
	UE_LOG(LogTemp, Warning, TEXT("add loading"));
	const UAZDeveloperSettings* Settings = GetDefault<UAZDeveloperSettings>();
	if (!Settings || !Settings->LoadingScreenClass->IsValidLowLevel()) return;
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWorld* World = GetWorld();

	UGameViewportClient* GameViewport = GI->GetGameViewportClient();
	if (!GameViewport) return;

	if (LoadingWidget.IsValid())
	{
		GameViewport->RemoveViewportWidgetContent(LoadingWidget.ToSharedRef());
	}
	else
	{
		UAZLoadingScreen* LoadingScreen = Cast<UAZLoadingScreen>(UUserWidget::CreateWidgetInstance(*GI, Settings->LoadingScreenClass, NAME_None));
		LoadingWidget = LoadingScreen->TakeWidget();
		UE_LOG(LogTemp, Warning, TEXT("add Screen: %p"), LoadingWidget.Get());
		const int32 RandomIndex = FMath::RandRange(0, Tips.Num() - 1);
		LoadingScreen->TipsText->SetText(Tips[RandomIndex]);
	}

	if (GameViewport)
		GameViewport->AddViewportWidgetContent(LoadingWidget.ToSharedRef(), 1000);
}

void UAZSceneSubsystem::HideLoadingScreen()
{
	UWorld* World = GetWorld();

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda(
		[this]()
		{
			if (LoadingWidget.IsValid())
			{
				UGameInstance* GI = GetGameInstance();
				if (!GI) return;
				UGameViewportClient* GameViewport = GI->GetGameViewportClient();
				if (!GameViewport) return;
				UE_LOG(LogTemp, Warning, TEXT("remove loading"));
				UE_LOG(LogTemp, Warning, TEXT("remove Screen: %p"), LoadingWidget.Get());
				GameViewport->RemoveViewportWidgetContent(LoadingWidget.ToSharedRef());
			}
		}
	), 0.5f, false);
}