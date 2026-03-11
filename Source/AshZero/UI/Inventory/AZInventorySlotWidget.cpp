// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/AZInventorySlotWidget.h"
#include "Components/SizeBox.h"

void UAZInventorySlotWidget::SetSlotSize(float TileSize)
{
	InventorySlotSizeBox->SetWidthOverride(TileSize);
	InventorySlotSizeBox->SetHeightOverride(TileSize);
}
