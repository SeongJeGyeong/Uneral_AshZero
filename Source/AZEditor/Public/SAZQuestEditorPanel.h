// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "../../AshZero/Util/AZDefine.h"
#include "Widgets/SCompoundWidget.h"

class IDetailsView;
class UAZQuestTagEditorObject;
class UAZQuestCreationObject;
class SGameplayTagPicker;

enum class EQuestEditorMode
{
	Create,
	Edit
};

struct FQuestListItem
{
	FName QuestID;
	FName QuestName;
};

class AZEDITOR_API SAZQuestEditorPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAZQuestEditorPanel)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	FReply OnOpenQuestCreateWindow();
	void OnQuestSelected(TSharedPtr<FQuestListItem> SelectedItem);

	UObject* LoadObjectiveMapAsset() const;

	void RefreshQuestList();
	TSharedRef<ITableRow> OnGenerateQuestRow(TSharedPtr<FQuestListItem> QuestItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OpenQuestEditor(EQuestEditorMode Mode, FName QuestRowName = NAME_None);

	FReply OnCreateQuest();
	FReply OnModifyQuest();
	void LoadQuestDataToEditorObject(FName RowName);
	void LoadQuestDialog(UDataTable* DialogTable, FName TagName, EDialogType Type);
	void SaveQuestDialog(UDataTable* DialogTable, FName TagName, EDialogType Type);

	bool AlertQuestCreateMessage();
	void ShowCreateNotification(const FString& Message, bool bSuccessed = true);
	void CloseQuestCreator();

private:
	TSharedPtr<IDetailsView> TagMappingDetailsView;
	TSharedPtr<SGameplayTagPicker> GameplayTagPicker;
	TSharedPtr<SWindow> QuestCreateWindow;

	TObjectPtr<UAZQuestTagEditorObject> TagEditorObject;
	TObjectPtr<UAZQuestCreationObject> QuestEditorObject;

	TArray<FGameplayTagContainer> TagContainers;

	UDataTable* QuestDataTable;

	TArray<TSharedPtr<FQuestListItem>> QuestRows;
	TSharedPtr<SListView<TSharedPtr<FQuestListItem>>> QuestListView;
	FName PrevTag = NAME_None;

	FAZQuest SetQuestInfo();
};
