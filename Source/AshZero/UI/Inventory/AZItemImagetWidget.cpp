// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/AZItemImagetWidget.h"
#include "Components/Image.h"
#include "UI/HUD/AZHUD.h"

void UAZItemImagetWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (ID != 0)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AAZHUD* HUD = Cast<AAZHUD>(PC->GetHUD());
			if (HUD)
			{
				HUD->ShowTooltipByID(ID);
			}
		}
	}
}

void UAZItemImagetWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (ID != 0)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AAZHUD* HUD = Cast<AAZHUD>(PC->GetHUD());
			if (HUD)
			{
				HUD->HideTooltip();
			}
		}
	}
}

void UAZItemImagetWidget::SetUp(int32 NewID, UMaterialInstance* MI)
{
	ID = NewID;
	if (ItemImage && MI)
	{
		ItemImage->SetBrushFromMaterial(MI);
	}
}
