// Fill out your copyright notice in the Description page of Project Settings.


#include "AZQuestTagEditorObject.h"
#include "GameplayTagsManager.h"

void UAZQuestTagEditorObject::GetQuestTags(TArray<FGameplayTag>& OutTags)
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    FGameplayTag QuestRoot = Manager.RequestGameplayTag(FName("Quest"), false);

    if (QuestRoot.IsValid())
    {
        FGameplayTagContainer Children =
            Manager.RequestGameplayTagChildren(QuestRoot);

        for (const FGameplayTag& Tag : Children)
        {
            UE_LOG(LogTemp, Log, TEXT("Quest Tag: %s"), *Tag.GetTagName().ToString());
        }
    }
}