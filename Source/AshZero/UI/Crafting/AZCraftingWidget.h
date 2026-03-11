// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZCraftingWidget.generated.h"

class UScrollBox;
class UButton;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZCraftingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UScrollBox> CraftingListScroll;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UButton> CloseButton;
protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnCloseButtonClicked();
};
