// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/AZDefine.h"
#include "AZStoreWidget.generated.h"

class UAZInventoryGridWidget;
class UButton;
class UScrollBox;
class UTextBlock;
class UAZStoreItemSlotWidget;
class UAZQuantitySelectorWidget;
struct FAZStoreDataTableRow;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZStoreWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAZInventoryGridWidget> StashInventoryGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> StoreListScroll;

	void UpdateStoreList();

	UPROPERTY(EditAnywhere, Category = "AZ|Store")
	TSubclassOf<UAZStoreItemSlotWidget> ItemSlotWidgetClass;

	UPROPERTY(EditAnywhere, Category = "AZ|Store")
	TSubclassOf<UAZQuantitySelectorWidget> QuantitySelectorClass;
protected:
	UPROPERTY()
	TObjectPtr<UAZQuantitySelectorWidget> QuantitySelectorInstance;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrencyText;

	UFUNCTION()
	void OnQuantityConfirmed(int32 TargetID, int32 Quantity, EAZTradeMode TradeMode);
private:
	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void OnCurrencyChanged(int32 NewAmount);
public:
	void OnStoreSlotSelected(int32 TargetID, EAZTradeMode TradeMode);
};
