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

	void Construct(const FArguments& InArgs);

private:
	static constexpr const TCHAR* QuestTablePath = TEXT("/Game/Blueprints/Data/DataTables/DT_QuestList.DT_QuestList");
	static constexpr const TCHAR* DialogTablePath = TEXT("/Game/Blueprints/Data/DataTables/DT_Dialog.DT_Dialog");
	static constexpr const TCHAR* ObjectiveMapPath = TEXT("/Game/Blueprints/Data/DataAssets/DA_ObjectiveMap.DA_ObjectiveMap");

	static const TArray<EDialogType>& GetAllQuestDialogTypes();

	FReply OnOpenQuestCreateWindow();
	void OnQuestSelected(TSharedPtr<FQuestListItem> SelectedItem);

	void RefreshQuestList();
	TSharedRef<ITableRow> OnGenerateQuestRow(TSharedPtr<FQuestListItem> QuestItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OpenQuestEditor(EQuestEditorMode Mode, FName QuestRowName = NAME_None);

	FReply OnCreateQuest();
	FReply OnModifyQuest();
	FReply SaveQuestInternal(bool bIsModify);

	// 데이터 로드/저장
	UDataTable* LoadDialogTable() const;
	void LoadQuestDataToEditorObject(FName RowName);
	void LoadQuestDialog(UDataTable* DialogTable, FName TagName, EDialogType Type);
	void SaveQuestDialog(UDataTable* DialogTable, FName TagName, EDialogType Type);
	void SaveAllDialogs(UDataTable* DialogTable, FName TagName);
	void LoadAllDialogs(UDataTable* DialogTable, FName TagName);

	bool ValidateQuestData() const;
	void ShowNotification(const FString& Message, bool bSuccessed = true) const;
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

	FAZQuest SetQuestInfo() const;
};
