// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Util/AZDefine.h"
#include "AZDialogUIComponent.generated.h"

class UAZDialogBoxUI;
class AAZPlayerController;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZDialogUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAZDialogUIComponent();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAZDialogBoxUI> DialogBoxWidgetClass;
	UPROPERTY()
	TObjectPtr<UAZDialogBoxUI> DialogBoxWidget;

protected:
	virtual void BeginPlay() override;

public:	
	void ConstructUI();
	void SetDialogBox(ESlateVisibility Visibility);
	void OpenDialogBox(FGameplayTag GiverTag, FEnableButtons EnableButtons);

private:
	UPROPERTY()
	TObjectPtr<AAZPlayerController> Owner;

	UPROPERTY()
	TObjectPtr<AActor> OriginViewTarget;
};
