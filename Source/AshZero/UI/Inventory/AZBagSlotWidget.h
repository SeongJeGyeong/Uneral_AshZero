// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZBagSlotWidget.generated.h"

class UAZInventoryComponent;
class UImage;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZBagSlotWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
public:
	void InitBagSlot(UAZInventoryComponent* NewInventoryComponent);
	UPROPERTY()
	TObjectPtr<UAZInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UImage> ItemImage;

private:

};
