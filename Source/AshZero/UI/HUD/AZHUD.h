// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AZHUD.generated.h"

/**
 * 
 */
class UAZTooltipWidget;
class UAZItemBase;
class UAZStatusBarWidget;

UCLASS()
class ASHZERO_API AAZHUD : public AHUD
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
public:
	void ShowTooltip(UAZItemBase* Item);
	void ShowTooltipByID(int32 TargetID);
	void HideTooltip();

	bool IsTooltipWidgetVisibility();

	UPROPERTY(BlueprintReadWrite, Category = "AZ|UI")
	TObjectPtr<UAZStatusBarWidget> StatusBarWidget;

	void UpdateSpectatorTarget(AActor* NewViewTarget);
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAZTooltipWidget> TooltipWidgetClass;

	UPROPERTY()
	TObjectPtr<UAZTooltipWidget> TooltipWidgetInstance;
};
