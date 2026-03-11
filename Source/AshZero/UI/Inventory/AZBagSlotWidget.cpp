// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/AZBagSlotWidget.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Operation/AZDragDropOperation.h"
#include "Item/AZItemBase.h"
#include "Util/AZDefine.h"
#include "UI/Inventory/AZItemWidget.h"
#include "Components/AZInventoryComponent.h"
#include "Item/AZBagItem.h"
#include "Components/Border.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "Components/Image.h"
#include "Components/AZEquipmentComponent.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"

bool UAZBagSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	APlayerController* PC = GetOwningPlayer();

	AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC);
	if (AZPC == nullptr)
		return false;
	UAZDragDropOperation* AZInOperation = Cast<UAZDragDropOperation>(InOperation);
	if (AZInOperation == nullptr)
		return false;
	UWidget* DraggedWidget = AZInOperation->DefaultDragVisual;
	if (DraggedWidget == nullptr)
		return false;
	UAZItemWidget* DummyItemWidget = Cast<UAZItemWidget>(DraggedWidget);
	if (DummyItemWidget == nullptr)
		return false;

	UAZBagItem* NewBagItem = Cast<UAZBagItem>(AZInOperation->Payload);

	UAZEquipmentComponent* EquipComp = AZPC->EquipmentComp;
	if (!EquipComp) return false;

	if (NewBagItem != nullptr)
	{
		if (AZInOperation->SourceWidget)
		{
			//InventoryComponent->ChangeInventoryData(NewBagItem->InventoryComponent, AZInOperation->SourceWidget->InventoryComponent ,NewBagItem);
			EquipComp->EquipBagItem(NewBagItem, AZInOperation->SourceWidget->InventoryComponent);
			AZInOperation->SourceWidget->BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
			AZInOperation->SourceWidget->SetIconOpacity(1.0f);
			UMaterialInstance* SourceMaterial = AZInOperation->SourceWidget->ItemImage->GetDynamicMaterial();
			ItemImage->SetBrushFromMaterial(SourceMaterial);
			AZPC->SetItemDrag_Server(NewBagItem, false);

			if (UWorld* World = GetWorld())
			{
				UGameInstance* GI = World->GetGameInstance();
				if (GI == nullptr) return false;
				UAZSoundManagerSubsystem* SoundManger = GI->GetSubsystem<UAZSoundManagerSubsystem>();
				if (SoundManger == nullptr) return false;

				SoundManger->PlayUISFX(EUISFXType::Equip);
			}
		}
	}
	else
	{
		UAZItemBase* Item = Cast<UAZItemBase>(AZInOperation->Payload);
		if (Item == nullptr)
			return false;
		AZPC->SetItemDrag_Server(Item, false);
		Item->SetIsRotated(DummyItemWidget->GetIsRotatedOriginalItem());
		AZInOperation->SourceWidget->SetIconOpacity(1.0f);
		AZInOperation->SourceWidget->BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
		
	}
	return false;
}

void UAZBagSlotWidget::InitBagSlot(UAZInventoryComponent* NewInventoryComponent)
{
	if (NewInventoryComponent == nullptr)
		return;

	InventoryComponent = NewInventoryComponent;
	
	if (ItemImage == nullptr)
		return;


	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	UMaterialInstance* BagIcon = DataManger->GetMaterialByID(InventoryComponent->InventoryStruct.ID);
	if (BagIcon == nullptr)
	{
		return;
	}

	ItemImage->SetBrushFromMaterial(BagIcon);
	
}
