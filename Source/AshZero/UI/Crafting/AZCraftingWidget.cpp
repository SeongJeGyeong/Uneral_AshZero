// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Crafting/AZCraftingWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"

void UAZCraftingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UAZCraftingWidget::OnCloseButtonClicked);
		CloseButton->OnClicked.AddDynamic(this, &UAZCraftingWidget::OnCloseButtonClicked);
	}
}

void UAZCraftingWidget::OnCloseButtonClicked()
{
	SetVisibility(ESlateVisibility::Collapsed);

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly GameInputMode;
		PC->SetInputMode(GameInputMode);

		PC->bShowMouseCursor = false;
	}
}