// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Util/AZDefine.h"
#include "Misc/Optional.h"
#include "AZQuestManagerSubsystem.generated.h"

class AAZChest;

UCLASS(Blueprintable)
class ASHZERO_API UAZQuestManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void UpdateQuestProgress(const FGameplayTag& Tag, EQuestObjectiveType Type, int32 UpdateCount);
	FAZQuest* GetAvailableQuest(FGameplayTag NPCTag);
	bool IsCompletePrerequisiteQuests(FGameplayTag QuestTag);

	void AcceptQuest(FGameplayTag QuestTag);
	void AbandonQuest();

	const FAZQuest* GetCurrentQuest() const;
	void SetCurrentQuest(const FAZQuest& Quest) { CurrentQuest = Quest; }

	void CompleteQuest(FGameplayTag Tag);
	void CompleteCurrentQuest();
	void SpawnReward();

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAZChest> RewardBoxClass;

	UPROPERTY()
	TObjectPtr<UDataTable> QuestDataTable;

	// NPC, QuestTag
	TMap<FGameplayTag, TArray<FGameplayTag>> QuestTagListMap;

	// Tag, Quest
	TMap<FGameplayTag, FAZQuest> QuestMap;

	TOptional<FAZQuest> CurrentQuest;
};
