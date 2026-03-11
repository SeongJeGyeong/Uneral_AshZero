// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/AZDefine.h"
#include "AZQuestObjListItem.generated.h"

class UTextBlock;

UCLASS()
class ASHZERO_API UAZQuestObjListItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetObjectiveData(FAZQuestObjectiveData ObjectiveData);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> Objective;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> Count;
};
