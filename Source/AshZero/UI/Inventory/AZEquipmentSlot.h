// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/AZDefine.h"
#include "AZEquipmentSlot.generated.h"

class UImage;
class UAZItemWidget;
class UAZInventoryComponent;
class UAZItemBase;
class USizeBox;
class UTextBlock;
class UAZEquipmentComponent;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZEquipmentSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Config")
	EEquipmentSlot SlotType;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UImage> ItemImage;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UTextBlock> StackCount;

	void InitEquipmentSlot(UAZInventoryComponent* NewInventoryComponent, UAZEquipmentComponent* NewEquipmentComponent);

	UPROPERTY()
	TObjectPtr<UAZInventoryComponent> InventoryComponent;
	UPROPERTY()
	TObjectPtr<UAZEquipmentComponent> EquipmentComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Equipment")
	ESlotIndex SlotIndex = ESlotIndex::Slot_0;	//일반 장비슬롯이면 0, 무기 1번슬롯이면 0, 2번 슬롯이면 1; 

	void UnEquip();
private:
	TObjectPtr<UAZItemBase> DraggedItem = nullptr;
};
