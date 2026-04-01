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

    if (UObject* MappingAsset = LoadObject<UObject>(nullptr, ObjectiveMapPath))
        TagMappingDetailsView->SetObject(MappingAsset);
    else
        UE_LOG(LogTemp, Warning, TEXT("DA_ObjectiveMap not found"));

    ChildSlot
        [
            SNew(SScrollBox)

                + SScrollBox::Slot().Padding(10)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Quest Editor")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
                ]

                +SScrollBox::Slot().Padding(10)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Quest Gameplay Tags")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                ]

                + SScrollBox::Slot().Padding(10)
                [
                    SAssignNew(GameplayTagPicker, SGameplayTagPicker)
                        .TagContainers(TagContainers)
                        .ReadOnly(false)
                        .GameplayTagPickerMode(EGameplayTagPickerMode::ManagementMode)
                        .Filter(TEXT("Quest"))
                ]

                +SScrollBox::Slot().Padding(10)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Tag Name Mapping")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                ]

                + SScrollBox::Slot().Padding(10)
                [
                    TagMappingDetailsView.ToSharedRef()
                ]

                + SScrollBox::Slot().Padding(10)
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("Create Quest")))
                        .OnClicked(this, &SAZQuestEditorPanel::OnOpenQuestCreateWindow)
                ]

                + SScrollBox::Slot().Padding(10)
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

void SAZQuestEditorPanel::OpenQuestEditor(EQuestEditorMode Mode, FName QuestRowName)
{
    if (QuestEditorObject != nullptr)
    {
        if (QuestEditorObject->IsRooted()) QuestEditorObject->RemoveFromRoot();
        QuestEditorObject = nullptr;
    }

    QuestEditorObject = NewObject<UAZQuestCreationObject>();
    QuestEditorObject->AddToRoot();

    PrevTag = (Mode == EQuestEditorMode::Edit) ? QuestRowName : NAME_None;
    if (Mode == EQuestEditorMode::Edit)
        LoadQuestDataToEditorObject(QuestRowName);

    FText TitleText = (Mode == EQuestEditorMode::Create) 
        ? FText::FromString(TEXT("Create New Quest")) 
        : FText::FromString(TEXT("Modify Quest"));

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsArgs;
    DetailsArgs.bAllowSearch = false;
    DetailsArgs.bHideSelectionTip = true;

    TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsArgs);
    DetailsView->SetObject(QuestEditorObject);

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

    TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
    if (ParentWindow.IsValid())
    {
        // 모달 창으로 띄울 경우 부모 창은 모달 창이 사라지기 전까지 멈춤(코드 포함)
        FSlateApplication::Get().AddModalWindow(NewQuestWindow, ParentWindow);
    }
    else
    {
        // 부모를 못 찾을 경우 일반 창으로 (에디터 전체에 대해 모달)
        GEditor->EditorAddModalWindow(NewQuestWindow);
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

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

const TArray<EDialogType>& SAZQuestEditorPanel::GetAllQuestDialogTypes()
{
    static const TArray<EDialogType> Types = {
        EDialogType::BeforeAcceptQuest,
        EDialogType::AcceptQuest,
        EDialogType::DeclineQuest,
        EDialogType::InProgressQuest,
        EDialogType::CompletedQuest
    };
    return Types;
}

UDataTable* SAZQuestEditorPanel::LoadDialogTable() const
{
    return LoadObject<UDataTable>(nullptr, DialogTablePath);
}

FReply SAZQuestEditorPanel::OnOpenQuestCreateWindow()
{
    OpenQuestEditor(EQuestEditorMode::Create);
    return FReply::Handled();
}

void SAZQuestEditorPanel::OnQuestSelected(TSharedPtr<FQuestListItem> SelectedItem)
{
    if (SelectedItem.IsValid())
        OpenQuestEditor(EQuestEditorMode::Edit, SelectedItem->QuestID);
}

void SAZQuestEditorPanel::RefreshQuestList()
{
    QuestRows.Empty();

    if (!QuestDataTable)
        QuestDataTable = LoadObject<UDataTable>(nullptr, QuestTablePath);

    if (!QuestDataTable) return;

    for (const auto& Pair : QuestDataTable->GetRowMap())
    {
        if (Pair.Key.IsNone()) continue;

        const FAZQuest* Quest = reinterpret_cast<FAZQuest*>(Pair.Value);
        QuestRows.Emplace(MakeShared<FQuestListItem>(FQuestListItem{ Pair.Key, Quest->QuestName }));
    }

    if (QuestListView.IsValid())
        QuestListView->RequestListRefresh();
}

FReply SAZQuestEditorPanel::OnCreateQuest()
{
    return SaveQuestInternal(false);
}

FReply SAZQuestEditorPanel::OnModifyQuest()
{
    return SaveQuestInternal(true);
}

FReply SAZQuestEditorPanel::SaveQuestInternal(bool bIsModify)
{
    if (!QuestEditorObject || !QuestDataTable)
    {
        ShowNotification(TEXT("퀘스트 데이터가 없습니다."), false);
        return FReply::Handled();
    }

    UDataTable* DialogTable = LoadDialogTable();
    if (!DialogTable)
    {
        ShowNotification(TEXT("대화 데이터테이블을 찾을 수 없습니다."), false);
        return FReply::Handled();
    }

    if (!ValidateQuestData()) return FReply::Handled();
    const FName NewRowName = QuestEditorObject->QuestTag.GetTagName();

    if (bIsModify)
    {
        // 태그가 변경되었는데 이미 존재하는 경우
        if (PrevTag != NewRowName && QuestDataTable->GetRowNames().Contains(NewRowName))
        {
            ShowNotification(FString::Printf(TEXT("이미 존재하는 퀘스트 태그입니다. : %s"), *NewRowName.ToString()), false);
            return FReply::Handled();
        }
        QuestDataTable->RemoveRow(PrevTag);
    }
    else
    {
        if (QuestDataTable->GetRowNames().Contains(NewRowName))
        {
            ShowNotification(FString::Printf(TEXT("이미 존재하는 퀘스트 태그입니다. : %s"), *NewRowName.ToString()), false);
            return FReply::Handled();
        }
    }

    // 퀘스트 데이터 저장
    FAZQuest QuestRow = SetQuestInfo();
    QuestDataTable->AddRow(NewRowName, QuestRow);
    QuestDataTable->MarkPackageDirty();
    QuestDataTable->PostEditChange();

    // 대화 데이터 저장
    SaveAllDialogs(DialogTable, NewRowName);
    DialogTable->MarkPackageDirty();
    DialogTable->PostEditChange();

    // 완료 알림
    const FString Msg = bIsModify
        ? TEXT("퀘스트 수정 완료")
        : FString::Printf(TEXT("퀘스트 추가 완료 : %s"), *NewRowName.ToString());
    ShowNotification(Msg);

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

    UDataTable* DialogTable = LoadDialogTable();
    if (!DialogTable)
    {
        ShowNotification(TEXT("대화 데이터테이블을 찾을 수 없습니다."), false);
        return;
    }

    QuestEditorObject->QuestName = QuestRow->QuestName;
    QuestEditorObject->QuestTag = QuestRow->QuestTag;
    QuestEditorObject->QuestGiverTag = QuestRow->QuestGiverTag;
    QuestEditorObject->Description = QuestRow->Description;
    QuestEditorObject->PrerequisiteQuests = QuestRow->PrerequisiteQuests;
    QuestEditorObject->Objectives = QuestRow->Objectives;
    QuestEditorObject->Rewards = QuestRow->Rewards;

    LoadAllDialogs(DialogTable, QuestRow->QuestTag.GetTagName());
}

void SAZQuestEditorPanel::LoadQuestDialog(UDataTable* DialogTable, FName TagName, EDialogType Type)
{
    FName RowName = FName(*FString::Printf(TEXT("%s_%d"), *TagName.ToString(), static_cast<int32>(Type)));
    FAZDialog* Dialog = DialogTable->FindRow<FAZDialog>(RowName, TEXT("LoadDialog"));
    if (!Dialog) return;

    QuestEditorObject->GetDialogRowsForType(Type) = Dialog->DialogRows;
}

void SAZQuestEditorPanel::SaveQuestDialog(UDataTable* DialogTable, FName TagName, EDialogType Type)
{
    FName RowName = FName(*FString::Printf(TEXT("%s_%d"), *TagName.ToString(), static_cast<int32>(Type)));

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

void SAZQuestEditorPanel::SaveAllDialogs(UDataTable* DialogTable, FName TagName)
{
    for (const EDialogType Type : GetAllQuestDialogTypes())
        SaveQuestDialog(DialogTable, TagName, Type);
}

void SAZQuestEditorPanel::LoadAllDialogs(UDataTable* DialogTable, FName TagName)
{
    for (const EDialogType Type : GetAllQuestDialogTypes())
        LoadQuestDialog(DialogTable, TagName, Type);
}

bool SAZQuestEditorPanel::ValidateQuestData() const
{
    if (QuestEditorObject->QuestTag.GetTagName().IsNone())
    {
        ShowNotification(TEXT("퀘스트 태그는 'None'으로 설정할 수 없습니다."), false);
        return false;
    }

    if (QuestEditorObject->QuestGiverTag.GetTagName().IsNone())
    {
        ShowNotification(TEXT("퀘스트 부여자 태그는 'None'으로 설정할 수 없습니다."), false);
        return false;
    }

    if (QuestEditorObject->Objectives.IsEmpty())
    {
        ShowNotification(TEXT("최소 한 개 이상의 퀘스트 목표가 필요합니다."), false);
        return false;
    }

    for (const FAZQuestObjectiveData& ObjectiveData : QuestEditorObject->Objectives)
    {
        if (ObjectiveData.ObjectiveTag.GetTagName().IsNone())
        {
            ShowNotification(TEXT("퀘스트 목표 태그는 'None'으로 설정할 수 없습니다."), false);
            return false;
        }
    }

    return true;
}

void SAZQuestEditorPanel::ShowNotification(const FString& Message, bool bSuccessed) const
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

FAZQuest SAZQuestEditorPanel::SetQuestInfo() const
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