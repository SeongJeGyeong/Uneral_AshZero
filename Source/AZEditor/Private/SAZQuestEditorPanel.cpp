// Fill out your copyright notice in the Description page of Project Settings.


#include "SAZQuestEditorPanel.h"
#include "SlateOptMacros.h"
#include "GameplayTagsEditorModule.h"
#include "ISettingsModule.h"
#include "GameplayTagsEditor/Public/SGameplayTagPicker.h"
#include "SAddNewGameplayTagWidget.h"
#include "AZQuestTagEditorObject.h"
#include "AZQuestCreationObject.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAZQuestEditorPanel::Construct(const FArguments& InArgs)
{
    // Tag Editor UObject 생성
    TagEditorObject = NewObject<UAZQuestTagEditorObject>();
    if (!TagEditorObject->IsRooted())
    {
        TagEditorObject->AddToRoot(); // Editor GC 방지
    }

    TagContainers.Add(TagEditorObject->QuestTags);

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsArgs;
    DetailsArgs.bHideSelectionTip = true;
    DetailsArgs.bLockable = false;
    DetailsArgs.bAllowSearch = true;

    TagMappingDetailsView = PropertyEditorModule.CreateDetailView(DetailsArgs);

    if (UObject* MappingAsset = LoadObjectiveMapAsset())
    {
        TagMappingDetailsView->SetObject(MappingAsset);
    }

    ChildSlot
        [
            SNew(SScrollBox)

                + SScrollBox::Slot()
                .Padding(10)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Quest Editor")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
                ]

                +SScrollBox::Slot()
                .Padding(10)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Quest Gameplay Tags")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                ]

                + SScrollBox::Slot()
                .Padding(10)
                [
                    SAssignNew(GameplayTagPicker, SGameplayTagPicker)
                        .TagContainers(TagContainers)
                        .ReadOnly(false)
                        .GameplayTagPickerMode(EGameplayTagPickerMode::ManagementMode)
                        .Filter(TEXT("Quest"))
                ]

                +SScrollBox::Slot()
                .Padding(10)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Tag Name Mapping")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                ]

                + SScrollBox::Slot()
                .Padding(10)
                [
                    TagMappingDetailsView.ToSharedRef()
                ]

                + SScrollBox::Slot()
                .Padding(10)
                [
                    SNew(SButton)
                    .Text(FText::FromString(TEXT("Create Quest")))
                    .OnClicked(this, &SAZQuestEditorPanel::OnOpenQuestCreateWindow)
                ]

                + SScrollBox::Slot()
                .Padding(10)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Quest List")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                ]

                + SScrollBox::Slot()
                .Padding(10)
                [
                    SAssignNew(QuestListView, SListView<TSharedPtr<FQuestListItem>>)
                        .ItemHeight(24)
                        .ListItemsSource(&QuestRows)
                        .OnGenerateRow(this, &SAZQuestEditorPanel::OnGenerateQuestRow)
                        .OnMouseButtonDoubleClick(this, &SAZQuestEditorPanel::OnQuestSelected)
                ]
        ];

    RefreshQuestList();
}

FReply SAZQuestEditorPanel::OnOpenQuestCreateWindow()
{
    OpenQuestEditor(EQuestEditorMode::Create);

    return FReply::Handled();
}

void SAZQuestEditorPanel::OnQuestSelected(TSharedPtr<FQuestListItem> SelectedItem)
{
    if (!SelectedItem.IsValid())
        return;

    OpenQuestEditor(EQuestEditorMode::Edit, SelectedItem->QuestID);
}

UObject* SAZQuestEditorPanel::LoadObjectiveMapAsset() const
{
    const FString AssetPath = TEXT("/Game/Blueprints/Data/DataAssets/DA_ObjectiveMap.DA_ObjectiveMap");

    UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);

    if (!Asset)
    {
        UE_LOG(LogTemp, Warning, TEXT("DA_ObjectiveMap not found"));
    }

    return Asset;
}

void SAZQuestEditorPanel::RefreshQuestList()
{
    QuestRows.Empty();

    if (!QuestDataTable)
    {
        QuestDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Blueprints/Data/DataTables/DT_QuestList.DT_QuestList"));
    }

    if (!QuestDataTable)
    {
        return;
    }

    for (const auto& Pair : QuestDataTable->GetRowMap())
    {
        const FName RowName = Pair.Key;
        const FAZQuest* Quest = reinterpret_cast<FAZQuest*>(Pair.Value);

        if (RowName.IsNone()) continue;

        TSharedPtr<FQuestListItem> Item = MakeShared<FQuestListItem>();
        Item->QuestID = RowName;
        Item->QuestName = Quest->QuestName;
        QuestRows.Add(Item);
    }

    if (QuestListView.IsValid())
    {
        QuestListView->RequestListRefresh();
    }
}

TSharedRef<ITableRow> SAZQuestEditorPanel::OnGenerateQuestRow(TSharedPtr<FQuestListItem> QuestItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FQuestListItem>>, OwnerTable)
        [
            SNew(STextBlock)
                .Text(FText::Format(
                    FText::FromString("{0}  {1}"),
                    FText::FromName(QuestItem->QuestID),
                    FText::FromName(QuestItem->QuestName)
                ))
        ];
}

void SAZQuestEditorPanel::OpenQuestEditor(EQuestEditorMode Mode, FName QuestRowName)
{
    if (QuestEditorObject != nullptr)
    {
        if (QuestEditorObject->IsRooted()) QuestEditorObject->RemoveFromRoot();
        QuestEditorObject = nullptr;
    }

    // 1. 입력을 받을 데이터 객체 생성
    QuestEditorObject = NewObject<UAZQuestCreationObject>();
    if (!QuestEditorObject->IsRooted())
    {
        QuestEditorObject->AddToRoot(); //GC 방지
    }

    if (Mode == EQuestEditorMode::Edit)
    {
        PrevTag = QuestRowName;
        LoadQuestDataToEditorObject(QuestRowName);
    }
    else
    {
        PrevTag = NAME_None;
    }

    FText TitleText = (Mode == EQuestEditorMode::Create) ? 
        FText::FromString(TEXT("Create New Quest")) : FText::FromString(TEXT("Modify Quest"));

    // 2. 창 생성을 위한 위젯 구성
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsArgs;
    DetailsArgs.bAllowSearch = false;
    DetailsArgs.bHideSelectionTip = true;
    DetailsArgs.NotifyHook = nullptr;

    TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsArgs);
    DetailsView->SetObject(QuestEditorObject);

    // 3. 팝업 윈도우 생성
    TSharedRef<SWindow> NewQuestWindow = SNew(SWindow)
        .Title(TitleText)
        .ClientSize(FVector2D(500, 600))
        .SupportsMaximize(false)
        .SupportsMinimize(false)
        .HasCloseButton(true)
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    DetailsView.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(10)
                .HAlign(HAlign_Right)
                [
                    SNew(SButton)
                        .Text(TitleText)
                        .OnClicked(this, 
                            Mode == EQuestEditorMode::Create
                            ? &SAZQuestEditorPanel::OnCreateQuest
                            : &SAZQuestEditorPanel::OnModifyQuest)
                ]
        ];

    // 모달 창이 생성되기 전에 할당해야함
    QuestCreateWindow = NewQuestWindow;

    // 1. 부모 윈도우 찾기 (현재 패널이 속한 창)
    TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());

    // 모달 창으로 띄우기
    // 모달 창으로 띄울 경우 부모 창은 모달 창이 사라지기 전까지 멈춤(코드 포함)
    if (ParentWindow.IsValid())
    {
        FSlateApplication::Get().AddModalWindow(NewQuestWindow, ParentWindow);
    }
    else
    {
        // 부모를 못 찾을 경우 일반 창으로 (에디터 전체에 대해 모달)
        GEditor->EditorAddModalWindow(NewQuestWindow);
    }
}

FReply SAZQuestEditorPanel::OnCreateQuest()
{
    if (!QuestEditorObject || !QuestDataTable)
    {
        ShowCreateNotification(TEXT("퀘스트 데이터가 없습니다."), false);
        return FReply::Handled();
    }

    UDataTable* DialogTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Blueprints/Data/DataTables/DT_Dialog.DT_Dialog"));
    if (!DialogTable)
    {
        ShowCreateNotification(TEXT("대화 데이터테이블을 찾을 수 없습니다."), false);
        return FReply::Handled();
    }

    if (!AlertQuestCreateMessage()) return FReply::Handled();

    const FName NewRowName = QuestEditorObject->QuestTag.GetTagName();
    if (QuestDataTable->GetRowNames().Contains(NewRowName))
    {
        ShowCreateNotification(FString::Printf(TEXT("이미 존재하는 퀘스트 태그입니다. : %s"), *NewRowName.ToString()), false);
        return FReply::Handled();
    }


    FAZQuest NewRow = SetQuestInfo();

    QuestDataTable->AddRow(NewRowName, NewRow);
    QuestDataTable->MarkPackageDirty();
    QuestDataTable->PostEditChange();

    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::BeforeAcceptQuest);
    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::AcceptQuest);
    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::DeclineQuest);
    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::InProgressQuest);
    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::CompletedQuest);

    DialogTable->MarkPackageDirty();
    DialogTable->PostEditChange();

    ShowCreateNotification(FString::Printf(TEXT("퀘스트 추가 완료 : %s"), *NewRowName.ToString()));

    CloseQuestCreator();
    RefreshQuestList();
    return FReply::Handled();
}

FReply SAZQuestEditorPanel::OnModifyQuest()
{
    if (!QuestEditorObject || !QuestDataTable)
    {
        ShowCreateNotification(TEXT("퀘스트 데이터가 없습니다."), false);
        return FReply::Handled();
    }

    UDataTable* DialogTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Blueprints/Data/DataTables/DT_Dialog.DT_Dialog"));
    if (!DialogTable)
    {
        ShowCreateNotification(TEXT("대화 데이터테이블을 찾을 수 없습니다."), false);
        return FReply::Handled();
    }

    if (!AlertQuestCreateMessage()) return FReply::Handled();

    const FName NewRowName = QuestEditorObject->QuestTag.GetTagName();
    if (PrevTag != NewRowName && QuestDataTable->GetRowNames().Contains(NewRowName))
    {
        ShowCreateNotification(FString::Printf(TEXT("이미 존재하는 퀘스트 태그입니다. : %s"), *NewRowName.ToString()), false);
        return FReply::Handled();
    }

    FAZQuest UpdatedRow = SetQuestInfo();

    QuestDataTable->RemoveRow(PrevTag);
    QuestDataTable->AddRow(QuestEditorObject->QuestTag.GetTagName(), UpdatedRow);

    QuestDataTable->MarkPackageDirty();
    QuestDataTable->PostEditChange();

    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::BeforeAcceptQuest);
    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::AcceptQuest);
    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::DeclineQuest);
    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::InProgressQuest);
    SaveQuestDialog(DialogTable, QuestEditorObject->QuestTag.GetTagName(), EDialogType::CompletedQuest);

    DialogTable->MarkPackageDirty();
    DialogTable->PostEditChange();

    ShowCreateNotification(TEXT("퀘스트 수정 완료"), true);

    CloseQuestCreator();
    RefreshQuestList();
    PrevTag = NAME_None;

    return FReply::Handled();
}

void SAZQuestEditorPanel::LoadQuestDataToEditorObject(FName RowName)
{
    if (!QuestDataTable) return;

    FAZQuest* QuestRow = QuestDataTable->FindRow<FAZQuest>(RowName, TEXT("QuestEdit"));

    if (!QuestRow) return;

    UDataTable* DialogTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Blueprints/Data/DataTables/DT_Dialog.DT_Dialog"));
    if (!DialogTable)
    {
        ShowCreateNotification(TEXT("대화 데이터테이블을 찾을 수 없습니다."), false);
        return;
    }

    QuestEditorObject->QuestName = QuestRow->QuestName;
    QuestEditorObject->QuestTag = QuestRow->QuestTag;
    QuestEditorObject->QuestGiverTag = QuestRow->QuestGiverTag;
    QuestEditorObject->Description = QuestRow->Description;
    QuestEditorObject->PrerequisiteQuests = QuestRow->PrerequisiteQuests;
    QuestEditorObject->Objectives = QuestRow->Objectives;
    QuestEditorObject->Rewards = QuestRow->Rewards;

    LoadQuestDialog(DialogTable, QuestRow->QuestTag.GetTagName(), EDialogType::BeforeAcceptQuest);
    LoadQuestDialog(DialogTable, QuestRow->QuestTag.GetTagName(), EDialogType::AcceptQuest);
    LoadQuestDialog(DialogTable, QuestRow->QuestTag.GetTagName(), EDialogType::DeclineQuest);
    LoadQuestDialog(DialogTable, QuestRow->QuestTag.GetTagName(), EDialogType::InProgressQuest);
    LoadQuestDialog(DialogTable, QuestRow->QuestTag.GetTagName(), EDialogType::CompletedQuest);
}

void SAZQuestEditorPanel::LoadQuestDialog(UDataTable* DialogTable, FName TagName, EDialogType Type)
{
    FName RowName = FName(*FString::Printf(TEXT("%s_%d"), *TagName.ToString(), Type));
    FAZDialog* Dialog = DialogTable->FindRow<FAZDialog>(RowName, TEXT("LoadDialog"));
    if (!Dialog) return;

    switch (Type)
    {
    case EDialogType::NormalTalk:
        break;
    case EDialogType::BeforeAcceptQuest:
        QuestEditorObject->DialogsBeforeAccept = Dialog->DialogRows;
        break;
    case EDialogType::InProgressQuest:
        QuestEditorObject->DialogsInProgress = Dialog->DialogRows;
        break;
    case EDialogType::CompletedQuest:
        QuestEditorObject->DialogsAfterComplete = Dialog->DialogRows;
        break;
    case EDialogType::AcceptQuest:
        QuestEditorObject->DialogsAfterAccept = Dialog->DialogRows;
        break;
    case EDialogType::DeclineQuest:
        QuestEditorObject->DialogsAfterDecline = Dialog->DialogRows;
        break;
    default:
        break;
    }
}

void SAZQuestEditorPanel::SaveQuestDialog(UDataTable* DialogTable, FName TagName, EDialogType Type)
{
    FName RowName = FName(*FString::Printf(TEXT("%s_%d"), *TagName.ToString(), Type));

    TArray<FAZDialogRow> Rows = QuestEditorObject->GetDialogRowsForType(Type);
    if (Rows.Num() > 0)
    {
        FAZDialog Dialog;
        Dialog.ContextTag = QuestEditorObject->QuestTag;
        Dialog.ConversationNPC = QuestEditorObject->QuestGiverTag;
        Dialog.DialogType = Type;
        Dialog.DialogRows = Rows;
        DialogTable->RemoveRow(RowName);
        DialogTable->AddRow(RowName, Dialog);
    }
}

bool SAZQuestEditorPanel::AlertQuestCreateMessage()
{
    const FName NewRowName = QuestEditorObject->QuestTag.GetTagName();
    if (NewRowName.IsNone())
    {
        ShowCreateNotification(TEXT("퀘스트 태그는 'None'으로 설정할 수 없습니다."), false);
        return false;
    }

    if (QuestEditorObject->QuestGiverTag.GetTagName().IsNone())
    {
        ShowCreateNotification(TEXT("퀘스트 부여자 태그는 'None'으로 설정할 수 없습니다."), false);
        return false;
    }

    if (QuestEditorObject->Objectives.IsEmpty())
    {
        ShowCreateNotification(TEXT("최소 한 개 이상의 퀘스트 목표가 필요합니다."), false);
        return false;
    }
    else
    {
        for (FAZQuestObjectiveData ObjectiveData : QuestEditorObject->Objectives)
        {
            if (ObjectiveData.ObjectiveTag.GetTagName().IsNone())
            {
                ShowCreateNotification(TEXT("퀘스트 목표 태그는 'None'으로 설정할 수 없습니다."), false);
                return false;
            }
        }
    }

    return true;
}

void SAZQuestEditorPanel::ShowCreateNotification(const FString& Message, bool bSuccessed)
{
    FNotificationInfo Info(FText::FromString(Message));
    Info.ExpireDuration = 4.0f;
    Info.bFireAndForget = true;
    if (!bSuccessed) Info.Image = FAppStyle::GetBrush("Icons.Error");

    FSlateNotificationManager::Get().AddNotification(Info);
}

void SAZQuestEditorPanel::CloseQuestCreator()
{
    // GC 정리
    QuestEditorObject->RemoveFromRoot();
    QuestEditorObject = nullptr;

    if (QuestCreateWindow.IsValid())
    {
        QuestCreateWindow->RequestDestroyWindow();
        QuestCreateWindow.Reset();
    }
}

FAZQuest SAZQuestEditorPanel::SetQuestInfo()
{
    FAZQuest QuestRow;
    if (QuestEditorObject)
    {
        QuestRow.QuestName = QuestEditorObject->QuestName;
        QuestRow.QuestTag = QuestEditorObject->QuestTag;
        QuestRow.QuestGiverTag = QuestEditorObject->QuestGiverTag;
        QuestRow.Description = QuestEditorObject->Description;
        QuestRow.PrerequisiteQuests = QuestEditorObject->PrerequisiteQuests;
        QuestRow.Objectives = QuestEditorObject->Objectives;
        QuestRow.Rewards = QuestEditorObject->Rewards;
    }

    return QuestRow;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION