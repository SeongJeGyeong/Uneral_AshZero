// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/AZEquipmentSlot.h"
#include "UI/Inventory/AZItemWidget.h"
#include "Components/AZInventoryComponent.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Operation/AZDragDropOperation.h"
#include "Item/AZItemBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/AZEquipmentComponent.h"
#include "UI/HUD/AZHUD.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"

void UAZEquipmentSlot::InitEquipmentSlot(UAZInventoryComponent* NewInventoryComponent, UAZEquipmentComponent* NewEquipmentComponent)
{
	if (NewInventoryComponent == nullptr)
		return;
	if (NewEquipmentComponent == nullptr)
		return;
	InventoryComponent = NewInventoryComponent;
	EquipmentComponent = NewEquipmentComponent;

	if (ItemImage == nullptr)
		return;


	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	StackCount->SetVisibility(ESlateVisibility::Collapsed);
}


bool UAZEquipmentSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	UAZDragDropOperation* AZInOperation = Cast<UAZDragDropOperation>(InOperation);
	if (AZInOperation == nullptr)
		return false;
	UAZItemBase* Item = Cast<UAZItemBase>(AZInOperation->Payload);
	if (Item == nullptr)
		return false;
	if (Item->GetItemType() != EItemType::Equipment && Item->GetItemType() != EItemType::Weapon_Firearm && Item->GetItemType() != EItemType::Supplies && Item->GetItemType() != EItemType::Throwables)
		return false;
	if (SlotType != Item->GetEquipmentSlotType())
		return false;
	AAZPlayerController* AZPC = Cast<AAZPlayerController>(GetOwningPlayer());
	if (AZPC == nullptr)
	if (EquipmentComponent == nullptr)
		return false;
	AZInOperation->SlotIndex = SlotIndex;
	
	EquipmentComponent->EquipItem(SlotType, AZInOperation);

	if (UWorld* World = GetWorld())
	{
		UGameInstance* GI = World->GetGameInstance();
		if (GI == nullptr) return false;
		UAZSoundManagerSubsystem* SoundManger = GI->GetSubsystem<UAZSoundManagerSubsystem>();
		if (SoundManger == nullptr) return false;

		SoundManger->PlayUISFX(EUISFXType::Equip);
	}
	return true;
}

void UAZEquipmentSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	UAZItemBase* Item = EquipmentComponent->GetItemRefBySlot(SlotType, SlotIndex);
	if (Item == nullptr) return;
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

void UAZEquipmentSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

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

FReply UAZEquipmentSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (InventoryComponent && EquipmentComponent)
		{
			DraggedItem = nullptr;
			switch (SlotType)
			{
			case EEquipmentSlot::Helmet:	DraggedItem = EquipmentComponent->Helmet;		break;
			case EEquipmentSlot::Armor:		DraggedItem = EquipmentComponent->Armor;		break;
			case EEquipmentSlot::Gloves:	DraggedItem = EquipmentComponent->Gloves;		break;
			case EEquipmentSlot::Boots:		DraggedItem = EquipmentComponent->Boots;		break;
			case EEquipmentSlot::Weapon:
				DraggedItem = (SlotIndex == ESlotIndex::Slot_0 ? EquipmentComponent->FirstWeapon : EquipmentComponent->SecondWeapon); break;
				//if (SlotIndex == ESlotIndex::Slot_1) DraggedItem = EquipmentComponent->SecondWeapon; break;	
			case EEquipmentSlot::QuickSlot:
				switch (SlotIndex)
				{
				case ESlotIndex::Slot_1: DraggedItem = EquipmentComponent->QuickSlotItem_1; break;
				case ESlotIndex::Slot_2: DraggedItem = EquipmentComponent->QuickSlotItem_2; break;
				case ESlotIndex::Slot_3: DraggedItem = EquipmentComponent->QuickSlotItem_3; break;
				case ESlotIndex::Slot_4: DraggedItem = EquipmentComponent->QuickSlotItem_4; break;
				default:break;
				}
			default:
				break;
			}
			if (DraggedItem != nullptr && DraggedItem->bIsStarterItem == false)
			{
				return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
			}
			
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UAZEquipmentSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (InventoryComponent == nullptr)
		return;
	if (InventoryComponent->ItemWidgetClass == nullptr)
		return;
	if (DraggedItem == nullptr)
		return;
	UAZItemWidget* DragVisualWdiget = CreateWidget<UAZItemWidget>(this, InventoryComponent->ItemWidgetClass);
	if (DragVisualWdiget != nullptr)
		DragVisualWdiget->InitForLootBox(InventoryComponent, DraggedItem);

	UAZDragDropOperation* DragDropOperation = NewObject<UAZDragDropOperation>();
	DragDropOperation->SourceWidget = DragVisualWdiget;
	DragDropOperation->DefaultDragVisual = DragVisualWdiget;
	DragDropOperation->Pivot = EDragPivot::CenterCenter;
	DragDropOperation->Payload = DraggedItem;
	DragDropOperation->SourceEquipmentSlot = this;

	if (SlotType == EEquipmentSlot::Weapon && SlotIndex == ESlotIndex::Slot_1)
		DragDropOperation->ItemSourceType = EItemSourceType::SecondWeapon;
	else if (SlotType == EEquipmentSlot::QuickSlot)
	{
		switch (SlotIndex)
		{
		case ESlotIndex::Slot_1: DragDropOperation->ItemSourceType = EItemSourceType::QuickSlot_1; break;
		case ESlotIndex::Slot_2: DragDropOperation->ItemSourceType = EItemSourceType::QuickSlot_2; break;
		case ESlotIndex::Slot_3: DragDropOperation->ItemSourceType = EItemSourceType::QuickSlot_3; break;
		case ESlotIndex::Slot_4: DragDropOperation->ItemSourceType = EItemSourceType::QuickSlot_4; break;
		default:break;
		}
	}
	else
		DragDropOperation->ItemSourceType = EItemSourceType::Equipment;
	
	OutOperation = DragDropOperation;
}

void UAZEquipmentSlot::UnEquip()
{
	if (EquipmentComponent == nullptr)
		return;
	EquipmentComponent->UnEquipItem(SlotType, SlotIndex);

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;
	StackCount->SetVisibility(ESlateVisibility::Collapsed);
}