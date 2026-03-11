// Fill out your copyright notice in the Description page of Project Settings.


#include "System/AZDataManagerSubsystem.h"
#include "DataAsset/AZBagShapeDataAsset.h"
#include "DataTable/AZItemDataTableRow.h"
#include "DataTable/AZDropDataTable.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "DataTable/AZCraftingRecipeDataTableRow.h"
#include "DataTable/AZStoreDataTableRow.h"
#include "System/Settings/AZDeveloperSettings.h"

void UAZDataManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UAZDeveloperSettings* Settings = GetDefault<UAZDeveloperSettings>();
	if (!Settings) return;

	BagShapeDataAsset = Settings->BagShapeDataAsset.LoadSynchronous();
	LoadBagData();

	MiscDataTable = Settings->MiscDataTable.LoadSynchronous();
	LoadDataTableToMap(MiscDataTable);
	MaterialDataTable = Settings->MaterialDataTable.LoadSynchronous();
	LoadDataTableToMap(MaterialDataTable);
	SuppliesDataTable = Settings->SuppliesDataTable.LoadSynchronous();
	LoadDataTableToMap(SuppliesDataTable);
	ThrowablesDataTable = Settings->ThrowablesDataTable.LoadSynchronous();
	LoadDataTableToMap(ThrowablesDataTable);
	WeaponDataTable = Settings->WeaponDataTable.LoadSynchronous();
	LoadDataTableToMap(WeaponDataTable);
	EquipmentDataTable = Settings->EquipmentDataTable.LoadSynchronous();
	LoadDataTableToMap(EquipmentDataTable);
	StorageDataTable = Settings->StorageDataTable.LoadSynchronous();
	LoadDataTableToMap(StorageDataTable);
	QuestDataTable = Settings->QuestItemDataTable.LoadSynchronous();
	LoadDataTableToMap(QuestDataTable);

	BioDropTable = Settings->BioDropTable.LoadSynchronous();
	MechDropTable = Settings->MechDropTable.LoadSynchronous();
	LootBoxDropTable = Settings->LootBoxDropTable.LoadSynchronous();
	BossDropTable = Settings->BossDropTable.LoadSynchronous();
	
	CraftingRecipeDataTable = Settings->CraftingRecipeDataTable.LoadSynchronous();
	StoreDataTable = Settings->StoreDataTable.LoadSynchronous();
	InitializeStoreData();
}

void UAZDataManagerSubsystem::Deinitialize()
{
	DataMap.Empty();
	Super::Deinitialize();
}

const FAZBaseItemDataTable* UAZDataManagerSubsystem::GetItemDataByID(int32 TargetID) const
{
	return DataMap.FindRef(TargetID);
}

const FBagDefinition* UAZDataManagerSubsystem::GetBagShapeByName(FName Name)
{
	const FBagDefinition* FoundBag = BagShapeDataAsset->BagList.FindByPredicate([&](const FBagDefinition& Item) {
		return Item.BagName == Name;
		});
	return FoundBag;
}

const FBagDefinition* UAZDataManagerSubsystem::GetRandomBagShapeByID(int32 TargetID)
{

	if (TargetID == 603)
	{
		if (BasicBags.Num() == 0)
			return nullptr;
		int32 RandomIndex = FMath::RandRange(0, BasicBags.Num() - 1);
		return BasicBags[RandomIndex];
	}
	else if (TargetID == 604)
	{
		if (TacticalBags.Num() == 0)
			return nullptr;
		int32 RandomIndex = FMath::RandRange(0, TacticalBags.Num() - 1);
		return TacticalBags[RandomIndex];
	}
	else if (TargetID == 605)
	{
		if (SpecialBags.Num() == 0)
			return nullptr;
		int32 RandomIndex = FMath::RandRange(0, SpecialBags.Num() - 1);
		return SpecialBags[RandomIndex];
	}
	return nullptr;
}

EItemType UAZDataManagerSubsystem::GetItemTypeByID(int32 TargetID)
{
	const FAZBaseItemDataTable* FoundRow = GetItemDataByID(TargetID);
	if (FoundRow == nullptr)
		return EItemType::None;
	return FoundRow->ItemType;
}

int32 UAZDataManagerSubsystem::GetMaxStackSizeByID(int32 TargetID)
{
	const FAZBaseItemDataTable* FoundRow = GetItemDataByID(TargetID);
	if (FoundRow != nullptr)
		return FoundRow->MaxStack;
	return 1;
}

UMaterialInstance* UAZDataManagerSubsystem::GetMaterialByID(int32 ID)
{
	if (FAZBaseItemDataTable** FoundRowPtr = DataMap.Find(ID))
	{
		FAZBaseItemDataTable* ItemData = *FoundRowPtr;

		if (ItemData->MaterialAsset.IsPending())
		{
			return ItemData->MaterialAsset.LoadSynchronous();
		}
		return ItemData->MaterialAsset.Get();
	}
	return nullptr;
}

int32 UAZDataManagerSubsystem::GetIDByTag(FGameplayTag Tag)
{
	return ItemTagIDMap.FindRef(Tag);
}

const FAZBaseItemDataTable* UAZDataManagerSubsystem::GetItemDataByTag(FGameplayTag Tag) const
{
	int32 ID = ItemTagIDMap.FindRef(Tag);
	return DataMap.FindRef(ID);
}

const TArray<FAZDropDataTable*> UAZDataManagerSubsystem::GetDropTable(ELootSourceType LootSourceType)
{
	const FString ContextString(TEXT("DropTable Context"));
	TArray<FAZDropDataTable*> DataTableRows;
	switch (LootSourceType)
	{
	case ELootSourceType::BioMonster:
		if (BioDropTable)
			BioDropTable->GetAllRows(ContextString, DataTableRows);
		break;
	case ELootSourceType::MechMonster:
		if (MechDropTable)
			MechDropTable->GetAllRows(ContextString, DataTableRows);
		break;
	case ELootSourceType::LootBox:
		if (LootBoxDropTable)
			LootBoxDropTable->GetAllRows(ContextString, DataTableRows);
		break;
	case ELootSourceType::Boss:
		if (BossDropTable)
			BossDropTable->GetAllRows(ContextString, DataTableRows);
		break;
	default:
		break;
	}
	return DataTableRows;
}

const TArray<FAZCraftingRecipeDataTableRow*> UAZDataManagerSubsystem::GetAllCraftingRecipe()
{
	const FString ContextString(TEXT("Crafting Recipe Context"));
	TArray<FAZCraftingRecipeDataTableRow*> CraftingRecipeTableRows;
	if (CraftingRecipeDataTable)
	{
		CraftingRecipeDataTable->GetAllRows(ContextString, CraftingRecipeTableRows);
	}
	return CraftingRecipeTableRows;
}

const FAZStoreDataTableRow* UAZDataManagerSubsystem::GetStoreDataByID(int32 ID)
{
	if (FAZStoreDataTableRow** FoundRow = StoreDataCache.Find(ID))
	{
		return *FoundRow;
	}
	return nullptr;
}


void UAZDataManagerSubsystem::InitializeStoreData()
{
	if (!StoreDataTable) return;

	StoreDataCache.Empty();

	static const FString ContextString(TEXT("StoreDataCache"));
	TArray<FAZStoreDataTableRow*> AllRows;

	StoreDataTable->GetAllRows<FAZStoreDataTableRow>(ContextString, AllRows);

	for (FAZStoreDataTableRow* Row : AllRows)
	{
		if (Row)
		{
			StoreDataCache.Add(Row->ID, Row);
		}
	}
}

void UAZDataManagerSubsystem::LoadDataTableToMap(UDataTable* InDataTable)
{
	if (!InDataTable) return;

	for (const auto& RowPair : InDataTable->GetRowMap())
	{
		FAZBaseItemDataTable* ItemPointer = reinterpret_cast<FAZBaseItemDataTable*>(RowPair.Value);
		if (ItemPointer)
		{
			int32 ItemID = ItemPointer->ID;
			FGameplayTag Tag = ItemPointer->GamePlayTag;
			if (DataMap.Contains(ItemID) || ItemTagIDMap.Contains(Tag))
				continue;
			DataMap.Add(ItemID, ItemPointer);
			ItemTagIDMap.Add(Tag, ItemID);
		}
	}
}

void UAZDataManagerSubsystem::LoadBagData()
{
	if (BagShapeDataAsset == nullptr)
		return;
	BasicBags.Empty();
	TacticalBags.Empty();
	SpecialBags.Empty();

	for (const FBagDefinition& BagItem : BagShapeDataAsset->BagList)
	{
		if (BagItem.ID == 603)
		{
			BasicBags.Add(&BagItem);
		}
		else if (BagItem.ID == 604)
		{
			TacticalBags.Add(&BagItem);
		}
		else if (BagItem.ID == 605)
		{
			SpecialBags.Add(&BagItem);
		}
	}
}


TMap<int32, FAZStoreDataTableRow*> UAZDataManagerSubsystem::GetStoreDatas()
{
	return StoreDataCache;
}
