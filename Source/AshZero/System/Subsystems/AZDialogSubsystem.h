// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Util/AZDefine.h"
#include "AZDialogSubsystem.generated.h"

class UAZObjectiveTag;

UCLASS()
class ASHZERO_API UAZDialogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	TArray<FAZDialog>* GetNPCDialogList(FGameplayTag NPCTag);
	FText GetTagText(FGameplayTag Tag);

	FAZDialog GetQuestDialog(FGameplayTag NPCTag, FGameplayTag QuestTag, EDialogType DialogType);
	bool QuestDialogIsValid(FGameplayTag NPCTag, FGameplayTag QuestTag, EDialogType DialogType);

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY()
	TObjectPtr<UAZObjectiveTag> TagTextDataAsset;

	UPROPERTY()
	TObjectPtr<UDataTable> DialogDataTable;

	TMap<FGameplayTag, TArray<FAZDialog>> DialogMap;
};
