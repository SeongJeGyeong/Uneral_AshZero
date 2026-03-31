// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Subsystems/AZQuestManagerSubsystem.h"
#include "Interactables/AZChest.h"
#include "Kismet/GameplayStatics.h"
#include "System/Settings/AZDeveloperSettings.h"
#include "System/Subsystems/AZDialogSubsystem.h"
#include "Components/AZPlayerInventoryComponent.h"
#include "Components/AZStashComponent.h"
#include "System/Player/AZPlayerController.h"
#include "System/AZDataManagerSubsystem.h"
#include "DataTable/AZBaseItemDataTable.h"

void UAZQuestManagerSubsystem::UpdateQuestProgress(const FGameplayTag& Tag, EQuestObjectiveType Type, int32 UpdateCount)
{
	if (!CurrentQuest.IsSet()) return;

	FAZQuest& Quest = CurrentQuest.GetValue();
	int32 CompleteCount = 0;
	bool bUpdatedAny = false;

	for (FAZQuestObjectiveData& Objective : Quest.Objectives)
	{
		if (Objective.bCompleted)
		{
			++CompleteCount;
			continue;
		}

		if (Objective.Type != Type) continue;

		bool bIsTarget = (Objective.ObjectiveTag == Tag) || 
						 (Type == EQuestObjectiveType::Slay && Objective.ObjectiveTag.GetTagName() == FName("Monster"));

		if (bIsTarget)
		{
			Objective.CurrentCount = (Type == EQuestObjectiveType::Collect) ? UpdateCount : Objective.CurrentCount + UpdateCount;

			if (Objective.CurrentCount >= Objective.RequiredCount)
			{
				Objective.bCompleted = true;
				++CompleteCount;
			}
			bUpdatedAny = true;
		}
	}

	if (bUpdatedAny && Quest.Objectives.Num() == CompleteCount)
	{
		Quest.bIsComplete = true;

		UAZDialogSubsystem* DialogSystem = GetGameInstance()->GetSubsystem<UAZDialogSubsystem>();

		if (DialogSystem &&
			DialogSystem->GetQuestDialog(Quest.QuestGiverTag, Quest.QuestTag, EDialogType::CompletedQuest).DialogRows.Num() > 0)
		{
			CompleteCurrentQuest();
		}
	}
}

FAZQuest* UAZQuestManagerSubsystem::GetAvailableQuest(FGameplayTag NPCTag)
{
	const TArray<FGameplayTag>* TagList = QuestTagListMap.Find(NPCTag);
	if (!TagList) return nullptr;

	for (const FGameplayTag& Tag : *TagList)
	{
		if (!IsCompletePrerequisiteQuests(Tag)) continue;

		if (FAZQuest* FoundQuest = QuestMap.Find(Tag))
			return FoundQuest;
	}

	return nullptr;
}

bool UAZQuestManagerSubsystem::IsCompletePrerequisiteQuests(FGameplayTag QuestTag)
{
	const FAZQuest* Quest = QuestMap.Find(QuestTag);

	if (!Quest || Quest->bIsComplete) return false;
	if (Quest->PrerequisiteQuests.IsEmpty()) return true;

	TArray<FGameplayTag> Tags;
	Quest->PrerequisiteQuests.GetGameplayTagArray(Tags);
	for (const FGameplayTag& Tag : Tags)
	{
		const FAZQuest* PrereqQuest = QuestMap.Find(Tag);
		if (!PrereqQuest && !PrereqQuest->bIsComplete) return false;
	}

	return true;
}

void UAZQuestManagerSubsystem::AcceptQuest(FGameplayTag QuestTag)
{
	CurrentQuest.Emplace(QuestMap[QuestTag]);

	AAZPlayerController* AZPC = Cast<AAZPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!AZPC) return;
	UAZDataManagerSubsystem* DataSystem = GetGameInstance()->GetSubsystem<UAZDataManagerSubsystem>();
	if (!DataSystem) return;

	for (FAZQuestObjectiveData& Objective : CurrentQuest.GetValue().Objectives)
	{
		if (Objective.Type != EQuestObjectiveType::Collect) continue;

		int32 ID = DataSystem->GetIDByTag(Objective.ObjectiveTag);
		int32 TotalAmount = AZPC->InventoryComp->GetItemCount(ID) + AZPC->StashComp->GetTotalItemCount(ID);

		if (TotalAmount > 0)
			UpdateQuestProgress(Objective.ObjectiveTag, EQuestObjectiveType::Collect, TotalAmount);
	}
}

void UAZQuestManagerSubsystem::AbandonQuest()
{
	CurrentQuest.Reset();
}

const FAZQuest* UAZQuestManagerSubsystem::GetCurrentQuest() const
{
	return CurrentQuest.GetPtrOrNull();
}

void UAZQuestManagerSubsystem::CompleteQuest(FGameplayTag Tag)
{
	FAZQuest* Quest = QuestMap.Find(Tag);
	if (!Quest) return;
	AAZPlayerController* AZPC = Cast<AAZPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!AZPC) return;

	for (const FAZQuestObjectiveData& Objective : Quest->Objectives)
	{
		if (Objective.Type != EQuestObjectiveType::Collect) continue;

		int32 Remains = AZPC->InventoryComp->RemoveItemByTag(Objective.ObjectiveTag, Objective.RequiredCount);
		if (Remains > 0)
		{
			AZPC->StashComp->RemoveItemByTag(Objective.ObjectiveTag, Remains);
		}
	}

	Quest->bIsComplete = true;
}

void UAZQuestManagerSubsystem::CompleteCurrentQuest()
{
	if (CurrentQuest.IsSet())
	{
		CompleteQuest(CurrentQuest.GetValue().QuestTag);
		CurrentQuest.Reset();
	}
}

void UAZQuestManagerSubsystem::SpawnReward()
{
	if (!CurrentQuest.IsSet()) return;

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Pawn) return;
	const UAZDeveloperSettings* Settings = GetDefault<UAZDeveloperSettings>();
	if (!Settings || !Settings->RewardBoxClass->IsValidLowLevel()) return;

	FVector Start = Pawn->GetActorLocation();
	FVector End = Start - FVector(0.0f, 0.0f, 500.0f);

	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(Pawn);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_WorldStatic,
		TraceParams
	);

	FVector SpawnLocation = bHit ? HitResult.Location + FVector(0.0f, 0.0f, 5.0f) : Start;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AAZChest* Box = GetWorld()->SpawnActor<AAZChest>(Settings->RewardBoxClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
		Box->GenerateRewardBox(QuestMap[CurrentQuest.GetValue().QuestTag].Rewards);
}

void UAZQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UAZDeveloperSettings* Settings = GetDefault<UAZDeveloperSettings>();
	if (!Settings || Settings->QuestDataTable.IsNull()) return;

	TArray<FAZQuest*> AllRows;
	Settings->QuestDataTable.LoadSynchronous()->GetAllRows(TEXT("Load QuestList"), AllRows);

	for (FAZQuest* Row : AllRows)
	{
		if (!Row) continue;

		QuestTagListMap.FindOrAdd(Row->QuestGiverTag).Add(Row->QuestTag);
		QuestMap.Add(Row->QuestTag, *Row);
	}
}

void UAZQuestManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
