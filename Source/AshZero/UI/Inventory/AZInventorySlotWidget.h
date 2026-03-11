// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZInventorySlotWidget.generated.h"

class USizeBox;
class UCanvasPanel;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UCanvasPanel> Canvas;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<USizeBox> InventorySlotSizeBox;

	void SetSlotSize(float TileSize);
};
