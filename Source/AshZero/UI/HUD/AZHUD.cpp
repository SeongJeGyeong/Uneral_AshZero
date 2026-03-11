// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AZHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "Components/AZInventoryComponent.h"
#include "Util/AZDefine.h"
#include "DataAsset/AZBagShapeDataAsset.h"
#include "UI/Inventory/AZTooltipWidget.h"
#include "Components/Overlay.h"
#include "Character/AZPlayerCharacter.h"
#include "UI/HUD/AZStatusBarWidget.h"

void AAZHUD::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = GetOwningPlayerController();
	if (TooltipWidgetClass && PC)
	{
		TooltipWidgetInstance = CreateWidget<UAZTooltipWidget>(PC, TooltipWidgetClass);
		if (TooltipWidgetInstance)
		{
			TooltipWidgetInstance->AddToViewport(2000);
			TooltipWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void AAZHUD::ShowTooltip(UAZItemBase* Item)
{
	if (Item == nullptr || TooltipWidgetInstance == nullptr) return;

	if (TooltipWidgetInstance->UpdateToolTipInfo(Item))
	{
		TooltipWidgetInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	
}
void AAZHUD::ShowTooltipByID(int32 TargetID)
{
	if (TargetID == 0 || TooltipWidgetInstance == nullptr) return;

	if (TooltipWidgetInstance->UpdateToolTipInfo(TargetID))
	{
		TooltipWidgetInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

}
void AAZHUD::HideTooltip()
{
	if (TooltipWidgetInstance)
	{
		TooltipWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool AAZHUD::IsTooltipWidgetVisibility()
{
	if (TooltipWidgetInstance == nullptr) return false;
	
	return TooltipWidgetInstance->IsVisible();

}

void AAZHUD::UpdateSpectatorTarget(AActor* NewViewTarget)
{
	AAZPlayerCharacter* TargetCharacter = Cast<AAZPlayerCharacter>(NewViewTarget);

	if (StatusBarWidget)
	{
		StatusBarWidget->BindToTargetCharacter(TargetCharacter);
	}
}
