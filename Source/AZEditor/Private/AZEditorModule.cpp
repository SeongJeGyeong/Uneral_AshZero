#include "AZEditorModule.h"
#include "Modules/ModuleManager.h"

#include "PropertyEditorModule.h"
#include "AZBagShapeDetails.h"
//
#include "../Public/SAZQuestEditorPanel.h"
#include "ToolMenus.h"

static const FName QuestEditorTabName("QuestEditor");

IMPLEMENT_MODULE(FAZEditor, AZEditor);

void FAZEditor::StartupModule()
{
#if WITH_EDITOR

	//PropertyEditor 모듈을 메모리로 불러오기
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	//BagDefinition 이름을 가진 구조체를 발견하면 AZBagShapeDetails::MakeInstance함수를 실행
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"BagDefinition",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&AZBagShapeDetails::MakeInstance)
	);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(QuestEditorTabName,
		FOnSpawnTab::CreateRaw(this, &FAZEditor::SpawnQuestEditorTab)).SetDisplayName(FText::FromString("Quest Editor")).SetMenuType(ETabSpawnerMenuType::Hidden);

	RegisterMenus();
#endif
}

void FAZEditor::ShutdownModule()
{
#if WITH_EDITOR
	//PropertyEditor 모듈이 아직 메모리에 있는지 확인
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		// 메모리 누수 방지 및 안전한 종료를 위함
		PropertyModule.UnregisterCustomPropertyTypeLayout("BagDefinition");
	}
#endif
}

TSharedRef<SDockTab> FAZEditor::SpawnQuestEditorTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SAZQuestEditorPanel)
		];
}

void FAZEditor::RegisterMenus()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
		{
			UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");

			FToolMenuSection& Section = Menu->AddSection("QuestEditor", FText::FromString("CustomTool"));
			Section.AddMenuEntry(
				"OpenQuestEditor",
				FText::FromString("Quest Editor"),
				FText::FromString("Open Quest Editor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
					{
						FGlobalTabmanager::Get()->TryInvokeTab(QuestEditorTabName);
					}))
			);
		}));
}