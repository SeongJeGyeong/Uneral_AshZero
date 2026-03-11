// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/AZDefine.h"
#include "AZPartyIcon.generated.h"

class UWidgetSwitcher;

UCLASS()
class ASHZERO_API UAZPartyIcon : public UUserWidget
{
	GENERATED_BODY()
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetSwitcher> IconSwitcher;

public:
	void SwitchIconState(EPartyIconState IconState);

};
