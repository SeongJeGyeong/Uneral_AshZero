// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/AZInventoryGridWidget.h"
#include "UI/Inventory/AZItemWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Border.h"
#include "Components/AZInventoryComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Item/AZItemBase.h"
#include "Util/AZDefine.h"
#include "UI/Inventory/AZInventorySlotWidget.h"
#include "Components/PanelSlot.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Operation/AZDragDropOperation.h"
#include "UI/Inventory/AZEquipmentSlot.h"

void UAZInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAZInventoryGridWidget::Init(UAZInventoryComponent* InventoryComp)
{
	UE_LOG(LogTemp, Warning, TEXT("GridWidget Init"));
	if (InventoryComp == nullptr)
		return;
	InventoryComponent = InventoryComp;

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UAZInventoryGridWidget::OnInventoryUpdated);
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UAZInventoryGridWidget::OnInventoryUpdated);
	}

	Columns = InventoryComp->GetInventoryStruct().Columns;
	Rows = InventoryComp->GetInventoryStruct().Rows;
	//TileSize = InventoryComp->TileSize;

	SetIsFocusable(true);

	float NewWidth = Columns * InventoryComp->TileSize;
	float NewHeight = Rows * InventoryComp->TileSize;
	NewHeight = FMath::Min(NewHeight, InventoryComp->TileSize * 10);
	UCanvasPanelSlot* BorderAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridBorder);
	BorderAsCanvasSlot->SetSize(FVector2D(NewWidth, NewHeight));
}


void UAZInventoryGridWidget::CreateLineSegments()
{
}

void UAZInventoryGridWidget::CreateInvnetorySlot()
{
	if (ItemsCanvasPanel == nullptr)
	{
		return;
	}
	if (GridCanvasPanel)
	{
		GridCanvasPanel->ClearChildren();
	}
	int32 GridColumns = InventoryComponent->InventoryStruct.Columns;
	UE_LOG(LogTemp, Warning, TEXT("GridFlags.Num(): %d"), InventoryComponent->InventoryStruct.GridFlags.Num());
	FVector2D StartPoint{};
	FVector2D EndPoint{};
	for (int32 i = 0; i < InventoryComponent->InventoryStruct.GridFlags.Num(); i++)
	{
		if (InventoryComponent->InventoryStruct.GridFlags[i] == false)
		{
			continue;
		}
		UAZInventorySlotWidget* NewSlot = CreateWidget<UAZInventorySlotWidget>(this, SlotWidgetClass);
		if (NewSlot)
		{
			NewSlot->SetSlotSize(InventoryComponent->TileSize);
			UPanelSlot* NewPanelSlot = GridCanvasPanel->AddChild(NewSlot);

			UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(NewPanelSlot);
			if (CanvasPanelSlot)
			{
				
				int32 X = i % GridColumns * InventoryComponent->TileSize;
				int32 Y = i / GridColumns * InventoryComponent->TileSize;
				//UE_LOG(LogTemp, Warning, TEXT("Index: %d -> Pos: %d, %d"), i, X, Y);
				CanvasPanelSlot->SetSize(FVector2D(InventoryComponent->TileSize, InventoryComponent->TileSize));
				CanvasPanelSlot->SetPosition(FVector2D(X, Y));
			}
		}
		
	}
}

void UAZInventoryGridWidget::CreateInvnetorySlotByData(FBagDefinition InventoryStruct, float TileSize)
{
	int32 GridColumns = InventoryStruct.Columns;
	UE_LOG(LogTemp, Warning, TEXT("GridFlags.Num(): %d"), InventoryStruct.GridFlags.Num());
	FVector2D StartPoint{};
	FVector2D EndPoint{};
	for (int32 i = 0; i < InventoryStruct.GridFlags.Num(); i++)
	{
		if (InventoryStruct.GridFlags[i] == false)
		{
			continue;
		}
		UAZInventorySlotWidget* NewSlot = CreateWidget<UAZInventorySlotWidget>(this, SlotWidgetClass);
		if (NewSlot)
		{
			NewSlot->SetSlotSize(TileSize);
			UPanelSlot* NewPanelSlot = GridCanvasPanel->AddChild(NewSlot);

			UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(NewPanelSlot);
			if (CanvasPanelSlot)
			{

				int32 X = i % GridColumns * TileSize;
				int32 Y = i / GridColumns * TileSize;
				UE_LOG(LogTemp, Warning, TEXT("Index: %d -> Pos: %d, %d"), i, X, Y);
				CanvasPanelSlot->SetSize(FVector2D(TileSize, TileSize));
				CanvasPanelSlot->SetPosition(FVector2D(X, Y));
			}
		}
	}
}

void UAZInventoryGridWidget::Refresh()
{
	if (InventoryComponent->ItemWidgetClass)
	{
		UAZItemWidget* ItemWidget = CreateWidget<UAZItemWidget>(GetOwningPlayer(), InventoryComponent->ItemWidgetClass);
		//InventoryComponent->ItemWidget = Cast<UAZItemWidget>(CreateWidget(GetWorld(), InventoryComponent->ItemWidgetClass));
		//InventoryComponent->ItemWidget->SetOwningPlayer(GetOwningPlayer());
		ItemWidget->AddToViewport(0);

		int32 X = InventoryComponent->GetAllItems()[InventoryComponent->ItemToAdded].X * InventoryComponent->TileSize;
		int32 Y = InventoryComponent->GetAllItems()[InventoryComponent->ItemToAdded].Y * InventoryComponent->TileSize;

		PanelSlot = ItemsCanvasPanel->AddChild(InventoryComponent->ItemWidget);
		Cast<UCanvasPanelSlot>(PanelSlot)->SetAutoSize(true);
		Cast<UCanvasPanelSlot>(PanelSlot)->SetPosition(FVector2D(X, Y));

		InventoryComponent->ItemWidget->Init(InventoryComponent);
	}
}

void UAZInventoryGridWidget::CreateAllItemWidget(UAZInventoryComponent* InteractedInventoryComponent)
{
	InventoryComponent = InteractedInventoryComponent;
	if (ItemsCanvasPanel)
	{
		ItemsCanvasPanel->ClearChildren();
	}
	TArray<UAZItemBase*> keys;
	InventoryComponent->GetAllItems().GetKeys(keys);

	if (InventoryComponent->ItemWidgetClass)
	{
		UAZItemWidget* ItemWidget;
		for (UAZItemBase* AddedItem : keys)
		{
			ItemWidget = CreateWidget<UAZItemWidget>(GetOwningPlayer(), InventoryComponent->ItemWidgetClass);
			ItemWidget->AddToViewport(0);
			//ItemWidget = Cast<UAZItemWidget>(CreateWidget(GetWorld(), InventoryComponent->ItemWidgetClass));
			//ItemWidget->SetOwningPlayer(GetOwningPlayer());

			int32 X = InventoryComponent->GetAllItems()[AddedItem].X * InventoryComponent->TileSize;
			int32 Y = InventoryComponent->GetAllItems()[AddedItem].Y * InventoryComponent->TileSize;

			PanelSlot = ItemsCanvasPanel->AddChild(ItemWidget);
			Cast<UCanvasPanelSlot>(PanelSlot)->SetAutoSize(true);
			Cast<UCanvasPanelSlot>(PanelSlot)->SetPosition(FVector2D(X, Y));
			ItemWidget->InitForLootBox(InventoryComponent, AddedItem);
		}
	}
}

bool UAZInventoryGridWidget::IsCurrentInventoryComponent(UAZInventoryComponent* OtherInventoryComponent)
{
	if (InventoryComponent == OtherInventoryComponent)
	{
		return true;
	}
	return false;
}

void UAZInventoryGridWidget::OnInventoryUpdated()
{
	if (InventoryComponent)
	{
		// 틱에서 하던 작업을 여기서 수행
		//Refresh(); // 필요한 경우
		CreateAllItemWidget(InventoryComponent);
	}
}


//화면에 그림을 그리는 함수. 매프레임마다 실행된다.
int32 UAZInventoryGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	FPaintContext PaintContext(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	FLinearColor CustomColor(0.5f, 0.5f, 0.5f, 0.5f);
	FVector2D TopLeftCorner = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.0f, 0.0f));

	if (InventoryComponent == nullptr)
		return LayerId;

	if (bIsItemOverGrid)
	{
		UAZItemBase* Item = Cast<UAZItemBase>(DraggedPayload);
		if (IsRoomAvailableForPayload(Item))
		{
			DrawBackgroundBox(Item, FLinearColor(0.0, 1.0, 0.0, 0.25), AllottedGeometry, TopLeftCorner, OutDrawElements, LayerId);
		}
		else
		{
			DrawBackgroundBox(Item, FLinearColor(1.0, 0.0, 0.0, 0.25), AllottedGeometry, TopLeftCorner, OutDrawElements, LayerId);
		}
	}
	return FMath::Max(LayerId, Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled));;
}

void UAZInventoryGridWidget::DrawBackgroundBox(UAZItemBase* Item, FLinearColor MyTintColor, const FGeometry& AllottedGeometry, FVector2D TopLeftCorner, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	float CurrentScrollOffset = 0.0f;
	if (InventoryScrollBox != nullptr)
	{
		CurrentScrollOffset = InventoryScrollBox->GetScrollOffset();
	}

	//float GridPixelWidth = InventoryComponent->InventoryStruct.Columns * InventoryComponent->TileSize;
	//float GridPixelHeight = InventoryComponent->InventoryStruct.Rows * InventoryComponent->TileSize;
	FVector2D GridVisibleSize = GridBorder->GetCachedGeometry().GetLocalSize();

	FVector2D GridMin = TopLeftCorner;
	FVector2D GridMax = TopLeftCorner + GridVisibleSize;
	//FVector2D GridMax = TopLeftCorner + FVector2D(GridPixelWidth, GridPixelHeight);

	float ItemPixelWidth = Item->GetDimensions().X * InventoryComponent->TileSize;
	float ItemPixelHeight = Item->GetDimensions().Y * InventoryComponent->TileSize;

	float ItemVisualY = (DraggedItemTopLeftTile.Y * InventoryComponent->TileSize) - CurrentScrollOffset;

	//FVector2D ItemMin = TopLeftCorner + FVector2D(DraggedItemTopLeftTile.X * InventoryComponent->TileSize, DraggedItemTopLeftTile.Y * InventoryComponent->TileSize);
	FVector2D ItemMin = TopLeftCorner + FVector2D(DraggedItemTopLeftTile.X * InventoryComponent->TileSize, ItemVisualY);
	FVector2D ItemMax = ItemMin + FVector2D(ItemPixelWidth, ItemPixelHeight);

	//인벤토리랑 아이템의 크기를 비교해서 교집합인 부분을 구하는 로직
	float IntersectMinX = FMath::Max(GridMin.X, ItemMin.X);
	float IntersectMinY = FMath::Max(GridMin.Y, ItemMin.Y);
	float IntersectMaxX = FMath::Min(GridMax.X, ItemMax.X);
	float IntersectMaxY = FMath::Min(GridMax.Y, ItemMax.Y);


	if (IntersectMaxX > IntersectMinX && IntersectMaxY > IntersectMinY)
	{
		FSlateBrush BoxBrush;
		BoxBrush.DrawAs = ESlateBrushDrawType::Box;

		FVector2D ClippedSize(IntersectMaxX - IntersectMinX, IntersectMaxY - IntersectMinY);
		FVector2D ClippedPos(IntersectMinX, IntersectMinY);

		FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(ClippedSize,FSlateLayoutTransform(ClippedPos));

		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &BoxBrush, ESlateDrawEffect::None, MyTintColor);
	}
}

//드래그를 놓았을 때 실행
bool UAZInventoryGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// InGeometry: 드래그 되고 있는 이 위젯이 화면 상 위치와 크기에 대한 정보
	// InDragDropEvent: 마우스 커서의 절대 좌표
	// InOperation: 드래그 했던 정보가 담겨 있는 변수(payload)
	APlayerController* PC = GetOwningPlayer();
	AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC);
	if (AZPC == nullptr)
		return false;

	UAZItemWidget* DummyItemWidget = Cast<UAZItemWidget>(InOperation->DefaultDragVisual);
	if (DummyItemWidget == nullptr)
		return false;

	UAZDragDropOperation* AZInOperation = Cast<UAZDragDropOperation>(InOperation);
	if (AZInOperation->Payload)
	{
		DroppedItem = Cast<UAZItemBase>(AZInOperation->Payload);
		if (DroppedItem == nullptr)
			return false;

		int32 TopLeftIndex = InventoryComponent->TileToIndex(DraggedItemTopLeftTile);
		AZInOperation->SourceWidget->SetIconOpacity(1.0f);
		AZInOperation->SourceWidget->BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
		if (IsRoomAvailableForPayload(DroppedItem))
		{
			AZPC->MoveItem_Server(
				DummyItemWidget->InventoryComponent,
				InventoryComponent, 
				DroppedItem, 
				TopLeftIndex, 
				DroppedItem->GetIsRotated());
			
			if (AZInOperation->SourceEquipmentSlot)
			{
				AZInOperation->SourceEquipmentSlot->UnEquip();
			}
		}
		else
		{
			AZPC->SetItemDrag_Server(DroppedItem, false);
			DroppedItem->SetIsRotated(DummyItemWidget->GetIsRotatedOriginalItem());
		}
		
		bIsDropped = true;
		bIsItemOverGrid = false;
		return true;
	}
	
	
	return false;
}


//드래그를 하는 동안 현재 위젯의 위를 지나갈 때 실행. 위젯의 위에 있지만 가만히 있으면 실행X
bool UAZInventoryGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (InOperation == nullptr)
		return false;
	if (InventoryComponent == nullptr)
		return false;
	if (IsValid(InOperation->Payload))
	{
		DraggedItem = Cast<UAZItemBase>(InOperation->Payload);
		if (DraggedItem == nullptr)
		{
			return false;
		}
		FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
		FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);

		FVector2D GridStarterCoordinates = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.0f, 0.0f));
		FVector2D AdjustedPosition = LocalPosition - GridStarterCoordinates;

		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, FString::Printf(TEXT("X: %.2f Y: %.2f"), AdjustedPosition.X, AdjustedPosition.Y));

		if (InventoryScrollBox != nullptr)
		{
			AdjustedPosition.Y += InventoryScrollBox->GetScrollOffset();
		}

		FIntPoint ResultTile;
		
		FMousePositionInTile MousePositionInTile = MousePositionInTileResult(AdjustedPosition);

		if (MousePositionInTile.bRight)
		{
			ResultTile.X = FMath::Clamp(DraggedItem->GetDimensions().X - 1, 0, DraggedItem->GetDimensions().X - 1);
		}
		else
		{
			ResultTile.X = FMath::Clamp(DraggedItem->GetDimensions().X, 0, DraggedItem->GetDimensions().X);
		}

		if (MousePositionInTile.bDown)
		{
			ResultTile.Y = FMath::Clamp(DraggedItem->GetDimensions().Y - 1, 0, DraggedItem->GetDimensions().Y - 1);
		}
		else
		{
			ResultTile.Y = FMath::Clamp(DraggedItem->GetDimensions().Y, 0, DraggedItem->GetDimensions().Y);
		}

		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, FString::Printf(TEXT("Right: %s Down: %S"), Right ? TEXT("True") : TEXT("False"), Down ? TEXT("True") : TEXT("False")));
		DraggedItemTopLeftTile = FIntPoint(FMath::TruncToInt32(AdjustedPosition.X / InventoryComponent->TileSize), FMath::TruncToInt32(AdjustedPosition.Y / InventoryComponent->TileSize)) - (ResultTile / 2);
		return true;
	}
	return false;
}


// 버튼을 누르면 실행
FReply UAZInventoryGridWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::R)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("R is Clicked"));
		//return FReply::Handled();
		if (DraggedItem)
		{
			DraggedItem->RotateItem();
			if (StorredDragOperation)
			{
				UAZItemWidget* VisualDraggedItem = Cast<UAZItemWidget>(StorredDragOperation->DefaultDragVisual);
				if (VisualDraggedItem)
				{
					VisualDraggedItem->Refresh(DraggedItem);
				}
				return FReply::Handled();
			}
		}

	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}


// 드래그 중인 마우스 커서가 이 위젯의 영역 안으로 처음 진입했을 때 한 번 호출되는 함수
void UAZInventoryGridWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
	DraggedPayload = InOperation->Payload;
	bIsItemOverGrid = true;
	UDragDropOperation* DragOperation = Cast< UDragDropOperation>(InOperation);
	if (DragOperation)
	{
		StorredDragOperation = DragOperation;
	}
}

// 드래그 중인 마우스 커서가 이 위젯의 영역 밖으로 나갔을 때 호출
void UAZInventoryGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	
	DraggedPayload = nullptr;
	bIsItemOverGrid = false;

}

bool UAZInventoryGridWidget::IsRoomAvailableForPayload(UAZItemBase* Item) const
{
	if (Item)
	{
		return InventoryComponent->IsRoomAvailable(Item,InventoryComponent->TileToIndex(DraggedItemTopLeftTile));
	}
	return false;
}

FMousePositionInTile UAZInventoryGridWidget::MousePositionInTileResult(FVector2D MousePosition)
{
	FMousePositionInTile MousePositionInTile;
	// 마우스 커서가 현재 위치한 타일에서 오른쪽에 위치한지
	MousePositionInTile.bRight = fmod(MousePosition.X, InventoryComponent->TileSize) > (InventoryComponent->TileSize / 2);
	// 마우스 커서가 현재 위치한 타일에서 아래에 위치한지
	MousePositionInTile.bDown = fmod(MousePosition.Y, InventoryComponent->TileSize) > (InventoryComponent->TileSize / 2);

	return MousePositionInTile;
}



