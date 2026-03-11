// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZQuickSlotWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * 
 */
UCLASS()
class ASHZERO_API UAZQuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> ItemImage;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> StackCount;
};
