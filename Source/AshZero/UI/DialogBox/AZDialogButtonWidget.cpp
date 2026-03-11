// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DialogBox/AZDialogButtonWidget.h"
#include "Components/Button.h"

void UAZDialogButtonWidget::NativeConstruct()
{
	HandleOnClickedDelegate.BindUFunction(this, FName("HandleDialogButtonClicked"));
	SelectButton->OnClicked.Add(HandleOnClickedDelegate);

	Super::NativeConstruct();
}

void UAZDialogButtonWidget::HandleDialogButtonClicked()
{
	OnDialogButtonClicked.Broadcast(SelectButton);
}
