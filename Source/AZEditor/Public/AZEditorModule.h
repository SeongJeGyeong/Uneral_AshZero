#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FAZEditor : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedRef<SDockTab> SpawnQuestEditorTab(const FSpawnTabArgs& Args);
	void RegisterMenus();
};
