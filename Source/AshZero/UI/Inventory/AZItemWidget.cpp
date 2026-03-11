// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/AZItemWidget.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AZInventoryComponent.h"
#include "Components/AZBagInventoryComponent.h"
#include "Item/AZItemBase.h"
#include "Item/AZBagItem.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Operation/AZDragDropOperation.h"
#include "Components/TextBlock.h"
#include "UI/HUD/AZHUD.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"

void UAZItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UAZItemWidget::Init(UAZInventoryComponent* NewInventoryComponent) //InventoryGridWidget에서 Refresh할 때 사용
{
	if (NewInventoryComponent == nullptr)
		return;
	InventoryComponent = NewInventoryComponent;

	if (InventoryComponent->InventoryGridWidget->bIsDropped)
	{
		Refresh(InventoryComponent->InventoryGridWidget->DroppedItem);
	}
	else if (IsValid(InventoryComponent->ItemToAdded))
	{
		Refresh(InventoryComponent->ItemToAdded);
	}
}

void UAZItemWidget::InitForLootBox(UAZInventoryComponent* NewInventoryComponent, UAZItemBase* ItemToAdd)
{
	if (NewInventoryComponent == nullptr)
		return;
	InventoryComponent = NewInventoryComponent;

	Refresh(ItemToAdd);
}

void UAZItemWidget::Refresh(UAZItemBase* ItemToAdd)
{
	Item = ItemToAdd;
	if (ItemImage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemImage == nullptr"));
		return;
	}

	ItemImage->SetBrushFromMaterial(Item->GetIcon());
	Size = FVector2D(Item->GetDimensions().X * InventoryComponent->TileSize, Item->GetDimensions().Y * InventoryComponent->TileSize);
	
	UMaterialInstanceDynamic* DynamicMatInstance = ItemImage->GetDynamicMaterial();
	if (DynamicMatInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("DynamicMatInstance == nullptr"));
		return;
	}
	ItemImage->SetBrushFromMaterial(DynamicMatInstance);

	FName MaterialRotationName = TEXT("IconRotation");
	float NewRotationValue = 0.0f;

	if (Item->GetIsRotated())
	{
		NewRotationValue = 0.75f;
	}
	DynamicMatInstance->SetScalarParameterValue(MaterialRotationName, NewRotationValue);

	UCanvasPanelSlot* ImageAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemImage);

	ImageAsCanvasSlot->SetSize(Size);

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;
	if (DataManger->GetMaxStackSizeByID(Item->GetItemID()) > 1)
	{
		StackCount->SetVisibility(ESlateVisibility::HitTestInvisible);
		StackCount->SetText(FText::AsNumber(Item->GetStackCount()));
	}
	else
	{
		StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAZItemWidget::SetIconOpacity(float Value)
{
	if (ItemImage == nullptr)
	{
		return;
	}

	FName MaterialRotationName = TEXT("IconOpacity");
	UMaterialInstanceDynamic* DynamicMatInstance = ItemImage->GetDynamicMaterial();
	if (DynamicMatInstance == nullptr)
		return;
	DynamicMatInstance->SetScalarParameterValue(MaterialRotationName, Value);
}


void UAZItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bool bIsDragging = FSlateApplication::Get().IsDragDropping();
	if (bIsDragging)
	{
		return;
	}

	BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.2f));

	if (Item)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AAZHUD* HUD = Cast<AAZHUD>(PC->GetHUD());
			if (HUD)
			{
				HUD->ShowTooltip(Item);
			}
		}
	}

}

void UAZItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	bool bIsDragging = FSlateApplication::Get().IsDragDropping();
	if (bIsDragging)
	{
		return;
	}
	BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AAZHUD* HUD = Cast<AAZHUD>(PC->GetHUD());
		if (HUD)
		{
			HUD->HideTooltip();
		}
	}
}

//드래그 동작
void UAZItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	// InGeometry: 드래그 되고 있는 이 위젯이 화면 상 위치와 크기에 대한 정보
	// InMouseEvent: 입력 행위 자체에 대한 정보 (마우스 커서의 위치, 어떤 버튼을 눌렀는지 등)
	// OutOperation: 드래그 시스템에 데이터와 시각적 요소를 전달. 이 변수에 값을 지정해야 드래그가 실행. nullptr이면 실행X
	if (Item->GetIsDragged())
	{
		return;
	}
	
	InventoryComponent->SetDraggedItemLastIndex(InventoryComponent->GetIndexAtItem(Item));
	

	UAZDragDropOperation* DragOperation = NewObject<UAZDragDropOperation>();
	DragOperation->Payload = Item;				// 드래그 되고 있는 데이터. OnDrop에 전달할 정보
	DragOperation->ItemSourceType = EItemSourceType::Inventory;

	AAZPlayerController* AZPC = Cast<AAZPlayerController>(GetOwningPlayer());
	if (AZPC)
	{
		UAZItemWidget* DummyWidget = CreateWidget<UAZItemWidget>(AZPC, this->GetClass());
		if (DummyWidget)
		{
			DummyWidget->InventoryComponent = InventoryComponent;
			DummyWidget->Refresh(Item);
			DummyWidget->BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
			DummyWidget->bIsRotatedOriginalItem = Item->GetIsRotated();
			DragOperation->DefaultDragVisual = DummyWidget;
			DragOperation->SourceWidget = this;
		}
	}
	bIsRotatedOriginalItem = Item->GetIsRotated();
	BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.2f));
	SetIconOpacity(0.5f);
	AZPC->SetItemDrag_Server(Item, true);

	OutOperation = DragOperation;


	AAZHUD* HUD = Cast<AAZHUD>(AZPC->GetHUD());
	if (HUD)
	{
		HUD->HideTooltip();
	}
}

void UAZItemWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	InventoryComponent->InventoryGridWidget->bIsDropped = true;
	InventoryComponent->InventoryGridWidget->bIsItemOverGrid = false;
	AAZPlayerController* AZPC = Cast<AAZPlayerController>(GetOwningPlayer());
	BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
	SetIconOpacity(1.0f);
	Item->SetIsRotated(bIsRotatedOriginalItem);
	if (AZPC)
	{
		AZPC->SetItemDrag_Server(Item, false);
	}
}

//드래그 감지 시작
FReply UAZItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	//왼쪽 마우스 버튼으로 이 위젯을(TakeWidget) 드래그 시작하면 NativeOnDragDetected를 실행한다.
	//이 코드가 없으면 아무리 드래그를 해도 드래그가 실행되지 않음
}

FReply UAZItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (Item && Item->GetItemType() == EItemType::Storage)
		{
			UAZBagItem* BagItem = Cast<UAZBagItem>(Item);
			if (BagItem)
			{
				AAZPlayerController* AZPC = Cast<AAZPlayerController>(GetOwningPlayer());
				if (AZPC)
				{
					if (BagItem->InventoryComponent != nullptr)
					{
						BagItem->InventoryComponent->ToggleBagInventory(AZPC, BagItem->GetItemID());
					}
				}
			}
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

bool UAZItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	AAZPlayerController* AZPC = Cast<AAZPlayerController>(GetOwningPlayer());
	if (AZPC == nullptr)
		return false;

	UAZDragDropOperation* AZInOperation = Cast<UAZDragDropOperation>(InOperation);
	if (AZInOperation == nullptr)
		return false;

	if (AZInOperation->SourceWidget == nullptr)
		return false;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return false;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return false;

	UAZItemBase* DroppedItem = Cast<UAZItemBase>(AZInOperation->Payload);
	if (DroppedItem == nullptr)
		return false;
	if (DataManger->GetMaxStackSizeByID(DroppedItem->GetItemID()) <= 1)
		return false;
	if (AZInOperation->Payload)
	{
		AZPC->TryStackItem_Server( AZInOperation->SourceWidget->InventoryComponent, InventoryComponent, DroppedItem, Item,  AZInOperation->SourceWidget, this);
	}


	return false;
}


bool UAZItemWidget::GetIsRotatedOriginalItem()
{
	return bIsRotatedOriginalItem;
}


