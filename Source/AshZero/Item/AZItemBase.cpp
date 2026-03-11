// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AZItemBase.h"
#include "Net/UnrealNetwork.h"
#include "DataTable/AZItemDataTableRow.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "DataTable/AZEquipmentItemDataTableRow.h"
#include "DataTable/AZWeaponItemDataTableRow.h"
#include "DataTable/AZSuppliesItemDataTableRow.h"
#include "DataTable/AZThrowablesItemDataTable.h"

void UAZItemBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAZItemBase, ItemID);
	DOREPLIFETIME(UAZItemBase, StackCount);
	//DOREPLIFETIME_CONDITION(UAZItemBase, Dimensions, COND_InitialOnly);
	//DOREPLIFETIME_CONDITION(UAZItemBase, Icon, COND_InitialOnly);
	DOREPLIFETIME(UAZItemBase, bIsRotated);
	DOREPLIFETIME(UAZItemBase, bIsDragged);
	DOREPLIFETIME(UAZItemBase, bIsStarterItem);
}

void UAZItemBase::OnRep_ItemID()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	const FAZBaseItemDataTable* ItemData = DataManger->GetItemDataByID(ItemID);
	if (ItemData == nullptr)
		return;

	ApplyItemData(ItemData);
}

FIntPoint UAZItemBase::GetDimensions() const
{
	if (!bIsRotated)
		return Dimensions;
	else
		return FIntPoint(Dimensions.Y, Dimensions.X);
}

UMaterialInterface* UAZItemBase::GetIcon()
{
	return Icon;
}


bool UAZItemBase::GetIsRotated()
{
	return bIsRotated;
}

void UAZItemBase::SetIsRotated(bool bIsRotate)
{
	bIsRotated = bIsRotate;
}

bool UAZItemBase::GetIsDragged()
{
	return bIsDragged;
}

void UAZItemBase::SetIsDragged(bool bIsDrag)
{
	bIsDragged = bIsDrag;
}
void UAZItemBase::RotateItem()
{
	if (bIsRotated)
	{
		bIsRotated = false;
	}
	else
	{
		bIsRotated = true;
	}
}

void UAZItemBase::SetItemID(int32 NewItemID)
{
	ItemID = NewItemID;
	OnRep_ItemID();
}

int32 UAZItemBase::GetItemID()
{
	return ItemID;
}

void UAZItemBase::ApplyItemData(const FAZBaseItemDataTable* ItemData)
{
	if (ItemData == nullptr)
		return;

	Dimensions.X = ItemData->Columns;
	Dimensions.Y = ItemData->Rows;
	Icon = ItemData->MaterialAsset.LoadSynchronous();
	ItemType = ItemData->ItemType;

	switch (ItemData->ItemType)
	{
	case EItemType::Supplies:
	{
		const FAZSuppliesItemDataTableRow* SuppliesData = static_cast<const FAZSuppliesItemDataTableRow*>(ItemData);
		EquipSlot = SuppliesData->EquipmentSlot;
		break;
	}
	case EItemType::Throwables:
	{
		const FAZThrowablesItemDataTable* ThrowableData = static_cast<const FAZThrowablesItemDataTable*>(ItemData);
		EquipSlot = ThrowableData->EquipmentSlot;
		break;
	}
	case EItemType::Weapon_Firearm:
	{
		const FAZWeaponItemDataTableRow* WeaponData = static_cast<const FAZWeaponItemDataTableRow*>(ItemData);
		EquipSlot = WeaponData->EquipmentSlot;
		break;
	}
	case EItemType::Equipment:
	{
		const FAZEquipmentItemDataTableRow* EquipmentData = static_cast<const FAZEquipmentItemDataTableRow*>(ItemData);
		EquipSlot = EquipmentData->EquipmentSlot;
		break;
	}
	case EItemType::Storage:
		EquipSlot = EEquipmentSlot::Backpack;
		break;
	default:
		EquipSlot = EEquipmentSlot::None;
		break;
	}
	
}

void UAZItemBase::SetItemData(int32 NewItemID)
{
	//Dimensions.X = ItemData.Columns;
	//Dimensions.Y = ItemData.Rows;
	//Icon = ItemData.MaterialAsset.LoadSynchronous();
	//ItemType = ItemData.ItemType;
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	const FAZBaseItemDataTable* ItemData = DataManger->GetItemDataByID(NewItemID);
	if (ItemData == nullptr)
		return;

	ItemID = NewItemID;

	if (ItemData)
	{
		ApplyItemData(ItemData);
	}
}

EItemType UAZItemBase::GetItemType()
{
	return ItemType;
}

EEquipmentSlot UAZItemBase::GetEquipmentSlotType()
{
	return EquipSlot;
}

void UAZItemBase::SetStackCount(int32 NewStackCount)
{
	StackCount = NewStackCount;
}
int32 UAZItemBase::GetStackCount()
{
	return StackCount;
}