// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZDialogButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDialogButtonActivated, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDialogButtonClicked, UButton*);

UCLASS()
class ASHZERO_API UAZDialogButtonWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ButtonText;

	FOnDialogButtonActivated OnDialogButtonActivated;
	FOnDialogButtonClicked OnDialogButtonClicked;

private:
	FScriptDelegate HandleOnClickedDelegate;

private:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleDialogButtonClicked();
};
