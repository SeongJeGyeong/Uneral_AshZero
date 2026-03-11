// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Subsystems/AZStorageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Components/AZPlayerInventoryComponent.h"
#include "Components/AZEquipmentComponent.h"
#include "Components/AZStashComponent.h"
#include "Character/AZPlayerCharacter.h"
#include "System/Player/AZPlayerController.h"
#include "Item/AZItemBase.h"
#include "Item/AZBagItem.h"
#include "System/Subsystems/AZQuestManagerSubsystem.h"
#include "System/AZDataManagerSubsystem.h"
#include "DataTable/AZBaseItemDataTable.h"

void UAZStorageSubsystem::SavePlayerData(APlayerController* PC)
{
	if (PC == nullptr) return;
	AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC);
	if (AZPC == nullptr) return;
	FAZPlayerSaveData NewSaveData;

	//플레이어 인벤토리 저장
	UAZPlayerInventoryComponent* InventoryComponenet = AZPC->InventoryComp;
	if (InventoryComponenet)
	{
		InventoryComponenet->RefreshAllItems();
		NewSaveData.PlayerIventoryStruct = InventoryComponenet->InventoryStruct;

		UAZQuestManagerSubsystem* QuestSystem = GetGameInstance()->GetSubsystem<UAZQuestManagerSubsystem>();
		const FAZQuest* CurrentQuest = QuestSystem->GetCurrentQuest();
		TMap<FGameplayTag, int32> RequestMap;
		if (CurrentQuest)
		{
			for (const FAZQuestObjectiveData Objective : CurrentQuest->Objectives)
			{
				if (Objective.Type == EQuestObjectiveType::Collect)
					RequestMap.Add(Objective.ObjectiveTag, 0);
			}
		}
		for (auto Items : InventoryComponenet->AllItems)
		{
			if (Items.Key == nullptr) continue;
			FAZItemSaveData ItemData;
			ItemData.ItemId = Items.Key->GetItemID();
			ItemData.Count = Items.Key->GetStackCount();
			ItemData.Index = Items.Value;
			UAZBagItem* BagItem = Cast<UAZBagItem>(Items.Key);
			if (BagItem)
			{
				ItemData.bIsBagItem = true;
				ItemData.BagStruct = BagItem->GetBagData(); //
			}

			if (Items.Key->GetItemType() == EItemType::Quest)
			{
				UAZDataManagerSubsystem* DataSystem = GetGameInstance()->GetSubsystem<UAZDataManagerSubsystem>();
				const FAZBaseItemDataTable* QuestItem = DataSystem->GetItemDataByID(Items.Key->GetItemID());
				if (QuestItem && RequestMap.Contains(QuestItem->GamePlayTag))
				{
					RequestMap[QuestItem->GamePlayTag] += Items.Key->GetStackCount();
				}
			}

			NewSaveData.InventoryItems.Add(ItemData);
		}

		for (const TPair<FGameplayTag, int32>& Item : RequestMap)
		{
			QuestSystem->UpdateQuestProgress(Item.Key, EQuestObjectiveType::Collect, Item.Value);
		}
	}

	//창고 인벤토리 저장
	UAZStashComponent* StashComponent = AZPC->StashComp;
	if (StashComponent)
	{
		StashComponent->RefreshAllItems();
		NewSaveData.StashInventoryStruct = StashComponent->InventoryStruct;
		for (auto Items : StashComponent->AllItems)
		{
			if (Items.Key == nullptr) continue;
			FAZItemSaveData ItemData;
			ItemData.ItemId = Items.Key->GetItemID();
			ItemData.Count = Items.Key->GetStackCount();
			ItemData.Index = Items.Value;
			UAZBagItem* BagItem = Cast<UAZBagItem>(Items.Key);
			if (BagItem)
			{
				ItemData.bIsBagItem = true;
				ItemData.BagStruct = BagItem->GetBagData(); //
			}
			NewSaveData.StashItems.Add(ItemData);
		}
	}
	else
	{
		//창고 컴포넌트가 없으면 원래 있던 값으로 저장
		NewSaveData.StashItems = SavedPlayerData.StashItems;
		NewSaveData.StashInventoryStruct = StashComponent->InventoryStruct;
	}

	// 장착한 장비 아이템 저장
	UAZEquipmentComponent* EquipmentComponent = AZPC->EquipmentComp;
	if (EquipmentComponent)
	{
		TArray<UAZItemBase*> Items = { EquipmentComponent->Helmet, EquipmentComponent->Armor, EquipmentComponent->Gloves, EquipmentComponent->Boots, EquipmentComponent->Backpack, EquipmentComponent->SecondWeapon };
		for (auto Item : Items)
		{
			if (Item == nullptr) continue;
			FAZItemSaveData ItemData;
			ItemData.ItemId = Item->GetItemID();
			ItemData.Count = Item->GetStackCount();
			ItemData.Index = 1;
			UAZBagItem* BagItem = Cast<UAZBagItem>(Item);
			if (BagItem)
			{
				ItemData.bIsBagItem = true;
				ItemData.BagStruct = BagItem->GetBagData(); //
			}
			FAZEquipmentSavePair EquipmentSavePair;
			EquipmentSavePair.Slot = Item->GetEquipmentSlotType();
			EquipmentSavePair.ItemData = ItemData;
			NewSaveData.EquipmentItems.Add(EquipmentSavePair);
		}

		TArray<UAZItemBase*> SupplieItems = { EquipmentComponent->QuickSlotItem_1, EquipmentComponent->QuickSlotItem_2, EquipmentComponent->QuickSlotItem_3, EquipmentComponent->QuickSlotItem_4 };
		int32 Index = 0;
		for (auto Item : SupplieItems)
		{
			Index++;
			if (Item == nullptr) continue;
			FAZItemSaveData ItemData;
			ItemData.ItemId = Item->GetItemID();
			ItemData.Count = Item->GetStackCount();
			ItemData.Index = Index;

			FAZQuickSlotSavePair QuickSlotSavePair;
			QuickSlotSavePair.SlotIndex = Index;
			QuickSlotSavePair.ItemData = ItemData;
			NewSaveData.QuickSlotItems.Add(QuickSlotSavePair);
		}

	}

	SavedPlayerData = NewSaveData;

}

void UAZStorageSubsystem::LoadPlayerData(APlayerController* PC)
{
	if (PC == nullptr)
		return;
	AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC);
	if (AZPC == nullptr)
		return;

	if (AZPC->HasAuthority())
	{
		AZPC->ApplySaveData(SavedPlayerData);
	}
	else
	{
		AZPC->SyncPlayerData_Server(SavedPlayerData);
	}
}

void UAZStorageSubsystem::ClearAllItems(APlayerController* PC)
{

	SavedPlayerData.InventoryItems.Empty();
	UAZDataManagerSubsystem* DataSystem = GetGameInstance()->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataSystem)
	{
		const FBagDefinition* BagShape = DataSystem->GetBagShapeByName("603_1");
		SavedPlayerData.PlayerIventoryStruct = *BagShape;
	}

	SavedPlayerData.EquipmentItems.Empty();

	SavedPlayerData.QuickSlotItems.Empty();
}

