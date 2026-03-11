// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Chest/AZChestGridWidget.h"
#include "UI/Inventory/AZItemWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Border.h"
#include "Components/AZLootBoxComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Item/AZItemBase.h"
#include "Util/AZDefine.h"
#include "UI/Inventory/AZInventorySlotWidget.h"
#include "Components/AZInventoryComponent.h"
#include "Components/PanelSlot.h"

//void UAZChestGridWidget::NativeConstruct()
//{
//    Super::NativeConstruct();
//}
//
//int32 UAZChestGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
//{
//	Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
//	if (ChestComponent == nullptr)
//		return LayerId;
//
//
//	FPaintContext PaintContext(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
//	FLinearColor CustomColor(0.5f, 0.5f, 0.5f, 0.5f);
//	FVector2D TopLeftCorner = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.0f, 0.0f));
//
//	/*if (ChestComponent->LineStructData == nullptr)
//		return LayerId;
//
//	int32 k = 0;
//	for (int32 i = 0; i < ChestComponent->LineStructData->XLines.Num(); i++)
//	{
//
//		for (int32 j = 0; j < ChestComponent->LineStructData->YLines.Num(); j++)
//		{
//			k = i;
//		}
//		if (ChestComponent->StartX.IsValidIndex(i) && ChestComponent->StartY.IsValidIndex(k) && ChestComponent->EndX.IsValidIndex(i) && ChestComponent->EndY.IsValidIndex(k))
//		{
//			UWidgetBlueprintLibrary::DrawLine(PaintContext, FVector2D(ChestComponent->StartX[i], ChestComponent->StartY[k]) + TopLeftCorner, FVector2D(ChestComponent->EndX[i], ChestComponent->EndY[k]) + TopLeftCorner, CustomColor, false, 2.0f);
//		}
//
//	}*/
//
//	if (bIsItemOverGrid)
//	{
//		UAZItemBase* Item = Cast<UAZItemBase>(DraggedPayload);
//		if (IsRoomAvailableForPayload(Item))
//		{
//			DrawBackgroundBox(Item, FLinearColor(0.0, 1.0, 0.0, 0.25), AllottedGeometry, TopLeftCorner, OutDrawElements, LayerId);
//		}
//		else
//		{
//			DrawBackgroundBox(Item, FLinearColor(1.0, 0.0, 0.0, 0.25), AllottedGeometry, TopLeftCorner, OutDrawElements, LayerId);
//		}
//	}
//	return int32();
//}
//
//bool UAZChestGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
//{
//	if (InOperation->Payload)
//	{
//		DroppedItem = Cast<UAZItemBase>(InOperation->Payload);
//		if (DroppedItem == nullptr)
//			return false;
//
//	}
//    return false;
//}
//
//bool UAZChestGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
//{
//	if (InOperation == nullptr)
//		return false;
//	if (IsValid(InOperation->Payload))
//	{
//		DraggedItem = Cast<UAZItemBase>(InOperation->Payload);
//		if (DraggedItem == nullptr)
//		{
//			return false;
//		}
//		FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
//		FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);
//
//		FVector2D GridStarterCoordinates = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.0f, 0.0f));
//		FVector2D AdjustedPosition = LocalPosition - GridStarterCoordinates;
//
//		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, FString::Printf(TEXT("X: %.2f Y: %.2f"), AdjustedPosition.X, AdjustedPosition.Y));
//
//		FIntPoint ResultTile;
//
//		bool Down = MousePositionInTileResult(AdjustedPosition).Down;
//		bool Right = MousePositionInTileResult(AdjustedPosition).Right;
//
//		if (Right)
//		{
//			ResultTile.X = FMath::Clamp(DraggedItem->GetDimensions().X - 1, 0, DraggedItem->GetDimensions().X - 1);
//		}
//		else
//		{
//			ResultTile.X = FMath::Clamp(DraggedItem->GetDimensions().X, 0, DraggedItem->GetDimensions().X);
//		}
//
//		if (Down)
//		{
//			ResultTile.Y = FMath::Clamp(DraggedItem->GetDimensions().Y - 1, 0, DraggedItem->GetDimensions().Y - 1);
//		}
//		else
//		{
//			ResultTile.Y = FMath::Clamp(DraggedItem->GetDimensions().Y, 0, DraggedItem->GetDimensions().Y);
//		}
//
//		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, FString::Printf(TEXT("Right: %s Down: %S"), Right ? TEXT("True") : TEXT("False"), Down ? TEXT("True") : TEXT("False")));
//		DraggedItemTopLeftTile = FIntPoint(FMath::TruncToInt32(AdjustedPosition.X / ChestComponent->TileSize), FMath::TruncToInt32(AdjustedPosition.Y / ChestComponent->TileSize)) - (ResultTile / 2);
//		return true;
//	}
//	return false;
//}
//
//FReply UAZChestGridWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
//{
//    return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
//}
//
//void UAZChestGridWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
//{
//	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
//	DraggedPayload = InOperation->Payload;
//	bIsItemOverGrid = true;
//	UDragDropOperation* DragOperation = Cast< UDragDropOperation>(InOperation);
//	if (DragOperation)
//	{
//		StorredDragOperation = DragOperation;
//	}
//}
//
//void UAZChestGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
//{
//	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
//
//	DraggedPayload = nullptr;
//	bIsItemOverGrid = false;
//}
//
//
//void UAZChestGridWidget::DrawBackgroundBox(UAZItemBase* Item, FLinearColor MyTintColor, const FGeometry& AllottedGeometry, FVector2D TopLeftCorner, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
//{
//	float GridPixelWidth = ChestComponent->InventoryStruct.Columns * ChestComponent->TileSize;
//	float GridPixelHeight = ChestComponent->InventoryStruct.Rows * ChestComponent->TileSize;
//
//	FVector2D GridMin = TopLeftCorner;
//	FVector2D GridMax = TopLeftCorner + FVector2D(GridPixelWidth, GridPixelHeight);
//
//	float ItemPixelWidth = Item->GetDimensions().X * ChestComponent->TileSize;
//	float ItemPixelHeight = Item->GetDimensions().Y * ChestComponent->TileSize;
//
//	FVector2D ItemMin = TopLeftCorner + FVector2D(DraggedItemTopLeftTile.X * ChestComponent->TileSize, DraggedItemTopLeftTile.Y * ChestComponent->TileSize);
//	FVector2D ItemMax = ItemMin + FVector2D(ItemPixelWidth, ItemPixelHeight);
//
//	//인벤토리랑 아이템의 크기를 비교해서 교집합인 부분을 구하는 로직
//	float IntersectMinX = FMath::Max(GridMin.X, ItemMin.X);
//	float IntersectMinY = FMath::Max(GridMin.Y, ItemMin.Y);
//	float IntersectMaxX = FMath::Min(GridMax.X, ItemMax.X);
//	float IntersectMaxY = FMath::Min(GridMax.Y, ItemMax.Y);
//
//
//	if (IntersectMaxX > IntersectMinX && IntersectMaxY > IntersectMinY)
//	{
//		FSlateBrush BoxBrush;
//		BoxBrush.DrawAs = ESlateBrushDrawType::Box;
//
//		FVector2D ClippedSize(IntersectMaxX - IntersectMinX, IntersectMaxY - IntersectMinY);
//		FVector2D ClippedPos(IntersectMinX, IntersectMinY);
//
//		FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(ClippedSize, FSlateLayoutTransform(ClippedPos));
//
//		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &BoxBrush, ESlateDrawEffect::None, MyTintColor);
//	}
//}
//
//void UAZChestGridWidget::Refresh()
//{
//	TArray<UAZItemBase*> keys;
//	ChestComponent->GetAllItems().GetKeys(keys);
//
//	if (ChestComponent->ItemWidgetClass)
//	{
//		ChestComponent->ItemWidget = CreateWidget(GetWorld(), ChestComponent->ItemWidgetClass);
//		for (UAZItemBase* AddedItem : keys)
//		{
//
//			ChestComponent->ItemWidget->SetOwningPlayer(GetOwningPlayer());
//
//			int32 X = ChestComponent->GetAllItems()[AddedItem].X * ChestComponent->TileSize;
//			int32 Y = ChestComponent->GetAllItems()[AddedItem].Y * ChestComponent->TileSize;
//
//			PanelSlot = GridCanvasPanel->AddChild(ChestComponent->ItemWidget);
//			Cast<UCanvasPanelSlot>(PanelSlot)->SetAutoSize(true);
//			Cast<UCanvasPanelSlot>(PanelSlot)->SetPosition(FVector2D(X, Y));
//		}
//	}
//}
//
////void UAZChestGridWidget::OpenChest(UAZLootBoxComponent* ChestComponent)
////{
////	
////}
//
//bool UAZChestGridWidget::IsRoomAvailableForPayload(UAZItemBase* Item) const
//{
//	if (Item)
//	{
//		return ChestComponent->IsRoomAvailable(Item, ChestComponent->TileToIndex(DraggedItemTopLeftTile));
//	}
//	return false;
//}
//
//FMousePositionInTile UAZChestGridWidget::MousePositionInTileResult(FVector2D MousePosition)
//{
//	// 마우스 커서가 현재 위치한 타일에서 오른쪽에 위치한지
//	MousePositionInTile.Right = fmod(MousePosition.X, ChestComponent->TileSize) > (ChestComponent->TileSize / 2);
//	// 마우스 커서가 현재 위치한 타일에서 아래에 위치한지
//	MousePositionInTile.Down = fmod(MousePosition.Y, ChestComponent->TileSize) > (ChestComponent->TileSize / 2);
//
//	return MousePositionInTile;
//}