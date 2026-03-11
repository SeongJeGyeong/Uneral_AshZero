// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Util/AZDefine.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZChestGridWidget.generated.h"

/**
 * 
 */
class UCanvasPanel;
class UBorder;
class UAZLootBoxComponent;
class UPanelSlot;
class UAZItemBase;

UCLASS()
class ASHZERO_API UAZChestGridWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
//	virtual void NativeConstruct() override;
//	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
//	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
//	bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
//	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
//	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
//	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
//
//	void DrawBackgroundBox(UAZItemBase* Item, FLinearColor MyTintColor, const FGeometry& AllottedGeometry, FVector2D TopLeftCorner, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
//
//	UPROPERTY()
//	UObject* DraggedPayload;
//	UPROPERTY()
//	UDragDropOperation* StorredDragOperation;
//	bool IsRoomAvailableForPayload(UAZItemBase* Item) const;
//
//	FMousePositionInTile MousePositionInTile;
//
//	FMousePositionInTile MousePositionInTileResult(FVector2D MousePosition);
//
//	FIntPoint DraggedItemTopLeftTile;
//
//	bool bIsItemOverGrid;
//public:
//	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
//	TObjectPtr<UCanvasPanel> Canvas;
//
//	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
//	TObjectPtr<UBorder> GridBorder;
//
//	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
//	TObjectPtr<UCanvasPanel> GridCanvasPanel;
//
//	UPROPERTY()
//	TObjectPtr<UAZLootBoxComponent> ChestComponent;
//
//	UPROPERTY()
//	TObjectPtr<UPanelSlot> PanelSlot;
//
//
//	int32 Columns = 6;
//	int32 Rows = 9;
//	float TileSize;
//
//
//
//	UPROPERTY()
//	TObjectPtr<UAZItemBase> DroppedItem;
//
//	UPROPERTY()
//	TObjectPtr<UAZItemBase> DraggedItem;
//
//	bool bIsDropped = false;
//	void Refresh();
//
//	//void OpenChest(UAZLootBoxComponent* ChestComponent);
//
};
