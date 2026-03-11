// Fill out your copyright notice in the Description page of Project Settings.

#include "AZQuestCreationObject.h"

#if WITH_EDITOR
void UAZQuestCreationObject::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

    if (!Event.Property)
    {
        return;
    }

    const FName PropertyName = Event.Property->GetFName();

    // bHasChoices가 바뀌었을 때
    if (PropertyName == GET_MEMBER_NAME_CHECKED(FAZDialogRow, bHasChoices))
    {
        for (FAZDialogRow& Row : DialogsBeforeAccept)
        {
            if (!Row.bHasChoices && Row.Choices.Num() > 0)
            {
                Row.Choices.Empty();
            }
        }

        for (FAZDialogRow& Row : DialogsAfterAccept)
        {
            if (!Row.bHasChoices && Row.Choices.Num() > 0)
            {
                Row.Choices.Empty();
            }
        }

        for (FAZDialogRow& Row : DialogsAfterDecline)
        {
            if (!Row.bHasChoices && Row.Choices.Num() > 0)
            {
                Row.Choices.Empty();
            }
        }

        for (FAZDialogRow& Row : DialogsInProgress)
        {
            if (!Row.bHasChoices && Row.Choices.Num() > 0)
            {
                Row.Choices.Empty();
            }
        }

        for (FAZDialogRow& Row : DialogsAfterComplete)
        {
            if (!Row.bHasChoices && Row.Choices.Num() > 0)
            {
                Row.Choices.Empty();
            }
        }
    }
    if (PropertyName == GET_MEMBER_NAME_CHECKED(FAZDialogRow, Speaker))
    {
        for (FAZDialogRow& Row : DialogsBeforeAccept)
        {
            if (Row.Speaker != ESpeakerType::Custom)
            {
                Row.CustomSpeakerTag = FGameplayTag::EmptyTag;
            }
        }

        for (FAZDialogRow& Row : DialogsAfterAccept)
        {
            if (Row.Speaker != ESpeakerType::Custom)
            {
                Row.CustomSpeakerTag = FGameplayTag::EmptyTag;
            }
        }

        for (FAZDialogRow& Row : DialogsAfterDecline)
        {
            if (Row.Speaker != ESpeakerType::Custom)
            {
                Row.CustomSpeakerTag = FGameplayTag::EmptyTag;
            }
        }

        for (FAZDialogRow& Row : DialogsInProgress)
        {
            if (Row.Speaker != ESpeakerType::Custom)
            {
                Row.CustomSpeakerTag = FGameplayTag::EmptyTag;
            }
        }

        for (FAZDialogRow& Row : DialogsAfterComplete)
        {
            if (Row.Speaker != ESpeakerType::Custom)
            {
                Row.CustomSpeakerTag = FGameplayTag::EmptyTag;
            }
        }
    }
}

TArray<FAZDialogRow>& UAZQuestCreationObject::GetDialogRowsForType(EDialogType Type)
{
    switch (Type)
    {
    case EDialogType::BeforeAcceptQuest:
        return DialogsBeforeAccept;
    case EDialogType::InProgressQuest:
        return DialogsInProgress;
    case EDialogType::CompletedQuest:
        return DialogsAfterComplete;
    case EDialogType::AcceptQuest:
        return DialogsAfterAccept;
    case EDialogType::DeclineQuest:
        return DialogsAfterDecline;
    default:
        break;
    }

    return DialogsBeforeAccept;
}
#endif