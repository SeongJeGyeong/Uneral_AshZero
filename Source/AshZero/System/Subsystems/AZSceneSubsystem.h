// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Util/AZDefine.h"
#include "AZSceneSubsystem.generated.h"

class UMediaPlayer;
class UAZLoadingScreen;
class UAZSequenceDataAsset;
class ULevelSequence;
class ULevelSequencePlayer;
class AAZSequencePivot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCutsceneFinished);

UCLASS()
class ASHZERO_API UAZSceneSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool IsPIE(const UWorld* World);

	UFUNCTION(BlueprintCallable)
	void ShowLoadingScreen();

	UFUNCTION(BlueprintCallable)
	void HideLoadingScreen();

	//void PlayCutscene(ECutsceneType Type);
	//void StopCutscene();

	FOnCutsceneFinished OnCutsceneFinished;

	ULevelSequence* GetBossSequence(EBossType Type);

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AAZSequencePivot> SequencePivot;

	void PlayPlayerSequence();

	UFUNCTION()
	void FinishPlayerSequence();

private:
	UFUNCTION()
	void BeginLoadMap(const FWorldContext& WorldContext, const FString& MapName);
	UFUNCTION()
	void EndLoadMap(UWorld* InLoadedWorld);

	UFUNCTION()
	void BeginSeamlessTravel(UWorld* World, const FString& MapName);

	//UPROPERTY()
	//TObjectPtr<UAZLoadingScreen> LoadingScreen;
	UPROPERTY()
	TArray<FText> Tips;

	TSharedPtr<SWidget> LoadingWidget;

	UPROPERTY()
	TObjectPtr<UMediaPlayer> MediaPlayer;

	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentWidget;

	UPROPERTY(Transient)
	TObjectPtr<UAZSequenceDataAsset> SequenceMap;

	UPROPERTY(Transient)
	TObjectPtr<ULevelSequencePlayer> SequencePlayer;

	//UFUNCTION()
	//void OnCutsceneEnd();
};
