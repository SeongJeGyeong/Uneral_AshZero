
#include "UI/Level/Lobby/AZQuestObjListItem.h"
#include "Components/TextBlock.h"
#include "System/Subsystems/AZDialogSubsystem.h"

void UAZQuestObjListItem::SetObjectiveData(FAZQuestObjectiveData ObjectiveData)
{
	UAZDialogSubsystem* DialogSystem = GetGameInstance()->GetSubsystem<UAZDialogSubsystem>();
	if (!DialogSystem) return;

	FText ObjectiveText = DialogSystem->GetTagText(ObjectiveData.ObjectiveTag);
	
	switch (ObjectiveData.Type)
	{
	case EQuestObjectiveType::Slay:
		ObjectiveText = FText::Format(FText::FromString(TEXT("{0} 토벌")), ObjectiveText);
		break;
	case EQuestObjectiveType::Collect:
		ObjectiveText = FText::Format(FText::FromString(TEXT("{0} 수집")), ObjectiveText);
		break;
	case EQuestObjectiveType::Talk:
		ObjectiveText = FText::Format(FText::FromString(TEXT("{0} 대화")), ObjectiveText);
		break;
	case EQuestObjectiveType::ReachLocation:
		ObjectiveText = FText::Format(FText::FromString(TEXT("{0} 도달")), ObjectiveText);
		break;
	default:
		break;
	}

	Objective->SetText(ObjectiveText);
	Count->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), ObjectiveData.CurrentCount, ObjectiveData.RequiredCount)));
}
