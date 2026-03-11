// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "AZQuestTagEditorObject.generated.h"

// 모듈명_API 매크로가 없으면 다른 모듈의 DLL에서 심볼에 접근 불가
UCLASS()
class AZEDITOR_API UAZQuestTagEditorObject : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Quest")
    FGameplayTagContainer QuestTags;

    void GetQuestTags(TArray<FGameplayTag>& OutTags);
};