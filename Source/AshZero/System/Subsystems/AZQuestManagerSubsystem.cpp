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
#include "Components/AZPlayerInventoryComponent.h"
#include "Components/AZStashComponent.h"

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

		bool bIsTarget = (Objective.ObjectiveTag == Tag);
		bIsTarget = (Type == EQuestObjectiveType::Slay && Objective.ObjectiveTag.GetTagName() == FName("Monster"));

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
		if (UAZDialogSubsystem* DialogSystem = GetGameInstance()->GetSubsystem<UAZDialogSubsystem>())
		{
			FAZDialog Dialog = DialogSystem->GetQuestDialog(Quest.QuestGiverTag, Quest.QuestTag, EDialogType::CompletedQuest);
			if (Dialog.DialogRows.Num() <= 0)
			{
				CompleteCurrentQuest();
			}
		}
		else
		{
			CompleteCurrentQuest();
		}
	}
}

//TArray<FAZQuest*> UAZQuestManagerSubsystem::GetAvailableQuestList(FGameplayTag NPCTag)
//{
//	TArray<FAZQuest*> QuestList;
//	const TArray<FGameplayTag>* TagList = QuestTagListMap.Find(NPCTag);
//	if (!TagList) return QuestList;
//
//	QuestList.Reserve(TagList->Num());
//
//	for (const FGameplayTag& Tag : *TagList)
//	{
//		if (!IsCompletePrerequisiteQuests(Tag)) continue;
//
//		if (FAZQuest* FoundQuest = QuestMap.Find(Tag))
//			QuestList.Add(FoundQuest);
//	}
//
//	return QuestList;
//}

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
	if (QuestMap[QuestTag].bIsComplete) return false;
	if (QuestMap[QuestTag].PrerequisiteQuests.IsEmpty()) return true;

	TArray<FGameplayTag> Tags;
	QuestMap[QuestTag].PrerequisiteQuests.GetGameplayTagArray(Tags);
	for (const FGameplayTag& PrereqTag : Tags)
	{
		const FAZQuest* PrereqQuest = QuestMap.Find(PrereqTag);
		if (!PrereqQuest) return true;
		if (!PrereqQuest->bIsComplete) return false;
	}

	return true;
}

void UAZQuestManagerSubsystem::AcceptQuest(FGameplayTag QuestTag)
{
	CurrentQuest.Emplace(QuestMap[QuestTag]);

	TArray<FAZQuestObjectiveData>& Objectives = CurrentQuest.GetValue().Objectives;
	UAZDataManagerSubsystem* DataSystem = GetGameInstance()->GetSubsystem<UAZDataManagerSubsystem>();
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AAZPlayerController* AZPC = (PC) ? Cast<AAZPlayerController>(PC) : nullptr;	
	if (!AZPC) return;

	for (FAZQuestObjectiveData& Objective : Objectives)
	{
		if (Objective.Type != EQuestObjectiveType::Collect) continue;

		if (!DataSystem) continue;
		int32 ID = DataSystem->GetIDByTag(Objective.ObjectiveTag);
		int32 TotalAmount = 0;
		TotalAmount += AZPC->InventoryComp->GetItemCount(ID);
		TotalAmount += AZPC->StashComp->GetTotalItemCount(ID);
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
	if (CurrentQuest.IsSet())
	{
		return CurrentQuest.GetPtrOrNull();
	}

	return nullptr;
}

void UAZQuestManagerSubsystem::CompleteQuest(FGameplayTag Tag)
{
	FAZQuest* Quest = QuestMap.Find(Tag);
	if (!Quest) return;
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AAZPlayerController* AZPC = (PC) ? Cast<AAZPlayerController>(PC) : nullptr;
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

	if (QuestMap.Contains(Tag))
		QuestMap[Tag].bIsComplete = true;
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

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	APawn* Pawn = (PC) ? PC->GetPawn() : nullptr;
	if (!Pawn) return;

	const UAZDeveloperSettings* Settings = GetDefault<UAZDeveloperSettings>();
	if (!Settings || !Settings->RewardBoxClass->IsValidLowLevel()) return;

	FHitResult HitResult;
	FVector Start = Pawn->GetActorLocation();
	FVector End = Start - FVector(0.0f, 0.0f, 500.0f);
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(Pawn);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_WorldStatic,
		TraceParams
	);

	FVector SpawnLocation = bHit ? HitResult.Location + FVector(0.0f, 0.0f, 5.0f) : Pawn->GetActorLocation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AAZChest* Box = GetWorld()->SpawnActor<AAZChest>(Settings->RewardBoxClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
		Box->GenerateRewardBox(QuestMap[CurrentQuest.GetValue().QuestTag].Rewards);
}

void UAZQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString DataTablePath = TEXT("/Game/Blueprints/Data/DataTables/DT_QuestList.DT_QuestList");
	QuestDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DataTablePath));
	if (!QuestDataTable) return;

	const FString ContextString(TEXT("Load QuestList"));
	TArray<FAZQuest*> AllRows;
	QuestDataTable->GetAllRows(ContextString, AllRows);

	for (FAZQuest* Row : AllRows)
	{
		if (!Row) continue;

		QuestTagListMap.FindOrAdd(Row->QuestGiverTag).Add(Row->QuestTag);
		QuestMap.FindOrAdd(Row->QuestTag, *Row);
	}
}

void UAZQuestManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
