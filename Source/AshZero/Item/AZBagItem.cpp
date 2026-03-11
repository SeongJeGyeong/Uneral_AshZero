// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AZBagItem.h"
#include "System/AZDataManagerSubsystem.h"
#include "Components/AZBagInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "DataTable/AZItemDataTableRow.h"
#include "DataTable/AZSuppliesItemDataTableRow.h"
#include "DataTable/AZThrowablesItemDataTable.h"

void UAZBagItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAZBagItem, InventoryComponent);
	DOREPLIFETIME(UAZBagItem, BagData);
}

void UAZBagItem::OnRep_ItemID()
{
	Super::OnRep_ItemID();
}

void UAZBagItem::InitializeRandomBagData()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	// ItemID를 기반으로 랜덤 가방 모양 가져오기
	const FBagDefinition* BagDefinition = DataManger->GetRandomBagShapeByID(ItemID);
	if (BagDefinition == nullptr)
		return;
	
	BagData = *BagDefinition;
}


void UAZBagItem::ApplyItemData(const FAZBaseItemDataTable* ItemData)
{
	Super::ApplyItemData(ItemData);

	//if (ItemData == nullptr)
	//	return;

	//UGameInstance* GI = GetWorld()->GetGameInstance();
	//if (GI == nullptr)
	//	return;
	//UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	//if (DataManger == nullptr)
	//	return;

	///*FName BagName = ItemData->ItemName;
	//const FBagDefinition* BagDefinition = DataManger->GetBagShapeByName(BagName);*/
	//const FBagDefinition* BagDefinition = DataManger->GetRandomBagShapeByID(ItemID);
	//if (BagDefinition == nullptr)
	//	return;
	//BagData = *BagDefinition;
}

void UAZBagItem::InitBagItem(UAZBagInventoryComponent* NewInventoryComponent)
{
	if (NewInventoryComponent == nullptr)
		return;
	InventoryComponent = NewInventoryComponent;
	InventoryComponent->InitBagInventoryComponent(BagData);
}

FBagDefinition UAZBagItem::GetBagData()
{
	return BagData;
}

void UAZBagItem::SetBagData(FBagDefinition NewBagData)
{
	BagData = NewBagData;
}
