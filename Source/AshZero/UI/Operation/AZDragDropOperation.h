// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Util/AZDefine.h"
#include "AZDragDropOperation.generated.h"

class UAZItemWidget;
class UAZEquipmentSlot;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UAZItemWidget> SourceWidget;

	UPROPERTY()
	EItemSourceType ItemSourceType;

	UPROPERTY()
	TObjectPtr<UAZEquipmentSlot> SourceEquipmentSlot;

	UPROPERTY()
	ESlotIndex SlotIndex;
};
