// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZItemWidget.generated.h"

/**
 * 
 */
class UAZItemBase;
class UAZInventoryComponent;
class UCanvasPanel;
class UBorder;
class UImage;
class UTextBlock;
UCLASS()
class ASHZERO_API UAZItemWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UCanvasPanel> Canvas;

	//UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	//TObjectPtr<class USizeBox> BackgroundSizeBox;


	UPROPERTY()
	TObjectPtr<UAZItemBase> Item;

	FVector2D Size;

	bool bIsRotatedOriginalItem;
public:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> ItemImage;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UBorder> BackgroundBorder;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> StackCount;

	UPROPERTY()
	TObjectPtr<UAZInventoryComponent> InventoryComponent;

	void Init(UAZInventoryComponent* NewInventoryComponent);
	void InitForLootBox(UAZInventoryComponent* NewInventoryComponent, UAZItemBase* ItemToAdd);
	void Refresh(UAZItemBase* ItemToAdd);
	void SetIconOpacity(float Value);
	bool GetIsRotatedOriginalItem();
};
