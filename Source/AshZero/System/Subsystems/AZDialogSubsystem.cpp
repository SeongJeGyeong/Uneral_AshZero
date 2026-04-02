// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Subsystems/AZDialogSubsystem.h"
#include "DataAsset/AZObjectiveTag.h"
#include "System/Settings/AZDeveloperSettings.h"

void UAZDialogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UAZDeveloperSettings* Settings = GetDefault<UAZDeveloperSettings>();
	if (!Settings || Settings->DialogDataTable.IsNull() || Settings->ObjectiveData.IsNull()) return;

	//FString DataTablePath = TEXT("/Game/Blueprints/Data/DataTables/DT_Dialog.DT_Dialog");
	//DialogDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DataTablePath));
	//if (DialogDataTable)
	//{
	//	const FString ContextString(TEXT("Load DialogList"));
	//	TArray<FAZDialog*> AllRows;

	//	DialogDataTable->GetAllRows(ContextString, AllRows);

	//	for (FAZDialog* Row : AllRows)
	//	{
	//		if (Row)
	//		{
	//			DialogMap.FindOrAdd(Row->ConversationNPC).Add(*Row);
	//		}
	//	}
	//}
	DialogDataTable = Settings->DialogDataTable.LoadSynchronous();
	const FString ContextString(TEXT("Load DialogList"));
	TArray<FAZDialog*> AllRows;
	DialogDataTable->GetAllRows(ContextString, AllRows);

	for (FAZDialog* Row : AllRows)
	{
		if (Row) DialogMap.FindOrAdd(Row->ConversationNPC).Add(*Row);
	}

	TagTextDataAsset = Settings->ObjectiveData.LoadSynchronous();
}

void UAZDialogSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

TArray<FAZDialog>* UAZDialogSubsystem::GetNPCDialogList(FGameplayTag NPCTag)
{
	return DialogMap.Find(NPCTag);
}

FText UAZDialogSubsystem::GetTagText(FGameplayTag Tag)
{
	if (!TagTextDataAsset) return FText::FromString(TEXT("None"));

	FText* Text = TagTextDataAsset->ObjectiveNameMap.Find(Tag);
	return (Text != nullptr) ? *Text : FText::FromString(TEXT("None"));
}

FAZDialog UAZDialogSubsystem::GetQuestDialog(FGameplayTag NPCTag, FGameplayTag QuestTag, EDialogType DialogType)
{
	const FString RowString = FString::Printf(TEXT("%s_%d"), *QuestTag.GetTagName().ToString(), static_cast<int32>(DialogType));
	FAZDialog* Dialog = DialogDataTable ? DialogDataTable->FindRow<FAZDialog>(FName(*RowString), TEXT("FindQuestDialog")) : nullptr;

	return (Dialog != nullptr) ? *Dialog : FAZDialog();
}

bool UAZDialogSubsystem::QuestDialogIsValid(FGameplayTag NPCTag, FGameplayTag QuestTag, EDialogType DialogType)
{
	const FString RowString = FString::Printf(TEXT("%s_%d"), *QuestTag.GetTagName().ToString(), static_cast<int32>(DialogType));
	FAZDialog* Dialog = DialogDataTable ? DialogDataTable->FindRow<FAZDialog>(FName(*RowString), TEXT("FindQuestDialog")) : nullptr;
	return (Dialog && Dialog->DialogRows.Num() > 0);
}