// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "../../AshZero/Util/AZDefine.h"
#include "AZQuestCreationObject.generated.h"

UCLASS()
class AZEDITOR_API UAZQuestCreationObject : public UObject
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, Category = "Info", meta = (DisplayName = "퀘스트 제목"))
    FName QuestName;

    UPROPERTY(EditDefaultsOnly, Category = "Info", meta = (Categories = "Quest.ID", DisplayName = "퀘스트 태그"))
    FGameplayTag QuestTag; // 이 퀘스트의 고유 ID

    UPROPERTY(EditDefaultsOnly, Category = "Info", meta = (Categories = "NPC", DisplayName = "퀘스트 부여 NPC 태그"))
    FGameplayTag QuestGiverTag; // 퀘스트 부여자

    UPROPERTY(EditDefaultsOnly, Category = "Info", meta = (MultiLine, DisplayName = "설명"))
    FText Description;

    UPROPERTY(EditDefaultsOnly, Category = "Request", meta = (Categories = "Quest.ID", DisplayName = "선행 퀘스트 목록"))
    FGameplayTagContainer PrerequisiteQuests;

    UPROPERTY(EditAnywhere, Category = "Request", meta = (DisplayName = "퀘스트 목표"))
    TArray<FAZQuestObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, Category = "Reward", meta = (DisplayName = "보상 목록"))
    TArray<FAZQuestReward> Rewards;

    UPROPERTY(EditAnywhere, Category = "Dialog", meta = (DisplayName = "퀘스트 수락 전"))
    TArray<FAZDialogRow> DialogsBeforeAccept;

    UPROPERTY(EditAnywhere, Category = "Dialog", meta = (DisplayName = "퀘스트 수락"))
    TArray<FAZDialogRow> DialogsAfterAccept;

    UPROPERTY(EditAnywhere, Category = "Dialog", meta = (DisplayName = "퀘스트 거절"))
    TArray<FAZDialogRow> DialogsAfterDecline;

    UPROPERTY(EditAnywhere, Category = "Dialog", meta = (DisplayName = "퀘스트 진행중"))
    TArray<FAZDialogRow> DialogsInProgress;

    UPROPERTY(EditAnywhere, Category = "Dialog", meta = (DisplayName = "퀘스트 완료"))
    TArray<FAZDialogRow> DialogsAfterComplete;

    virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;

    TArray<FAZDialogRow>& GetDialogRowsForType(EDialogType Type);
};
