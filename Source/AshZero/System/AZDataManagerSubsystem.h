// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Util/AZDefine.h"
#include "AZDataManagerSubsystem.generated.h"

class UAZBagShapeDataAsset;
class UMaterialInstance;
struct FAZDropDataTable;
struct FAZItemDataTableRow;
struct FAZBaseItemDataTable;
struct FAZCraftingRecipeDataTableRow;
struct FAZStoreDataTableRow;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZDataManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//기본 데이터만 가져올 수 있음
	const FAZBaseItemDataTable* GetItemDataByID(int32 TargetID) const;
	template<typename T>

	//특정 타입의 아이템 데이터를 가져오는 방법
	//UGameInstance* GI = GetWorld()->GetGameInstance();
	//if (GI == nullptr)
	//	return;
	//UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	//if (DataManger == nullptr)
	//	return;
	//const FAZWeaponItemDataTableRow* WeaponData = DataManger->GetItemDataByID<FAZWeaponItemDataTableRow>(ItemToUse->GetItemID());
	const T* GetItemDataByID(int32 TargetID) const
	{
		const FAZBaseItemDataTable* BaseData = GetItemDataByID(TargetID);
		return static_cast<const T*>(BaseData);
	}

	const FBagDefinition* GetBagShapeByName(FName Name);
	const FBagDefinition* GetRandomBagShapeByID(int32 TargetID);
	EItemType GetItemTypeByID(int32 TargetID);
	int32 GetMaxStackSizeByID(int32 TargetID);
	UMaterialInstance* GetMaterialByID(int32 ID);
	int32 GetIDByTag(FGameplayTag Tag);
	const FAZBaseItemDataTable* GetItemDataByTag(FGameplayTag Tag) const;

	const TArray<FAZDropDataTable*> GetDropTable(ELootSourceType LootSourceType);
	const TArray<FAZCraftingRecipeDataTableRow*> GetAllCraftingRecipe();
	const FAZStoreDataTableRow* GetStoreDataByID(int32 ID);
	
	TMap<int32, FAZStoreDataTableRow*> GetStoreDatas();
private:
	UPROPERTY()
	TObjectPtr<UDataTable> SourceDataTable;

	UPROPERTY()
	TObjectPtr<UAZBagShapeDataAsset> BagShapeDataAsset;
	
	//아이템 데이터 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> MiscDataTable;
	UPROPERTY()
	TObjectPtr<UDataTable> MaterialDataTable;
	UPROPERTY()
	TObjectPtr<UDataTable> SuppliesDataTable;
	UPROPERTY()
	TObjectPtr<UDataTable> ThrowablesDataTable;
	UPROPERTY()
	TObjectPtr<UDataTable> WeaponDataTable;
	UPROPERTY()
	TObjectPtr<UDataTable> EquipmentDataTable;
	UPROPERTY()
	TObjectPtr<UDataTable> StorageDataTable;
	UPROPERTY()
	TObjectPtr<UDataTable> QuestDataTable;

	//드랍 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> BioDropTable;
	UPROPERTY()
	TObjectPtr<UDataTable> MechDropTable;
	UPROPERTY()
	TObjectPtr<UDataTable> LootBoxDropTable;
	UPROPERTY()
	TObjectPtr<UDataTable> BossDropTable;

	//제작 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> CraftingRecipeDataTable;

	//상점 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> StoreDataTable;
	TMap<int32, FAZStoreDataTableRow*> StoreDataCache;
	void InitializeStoreData();

	TMap<int32, FAZBaseItemDataTable*> DataMap;
	TMap<FGameplayTag, int32> ItemTagIDMap;
	
	TArray<const FBagDefinition*> BasicBags;
	TArray<const FBagDefinition*> TacticalBags;
	TArray<const FBagDefinition*> SpecialBags;

	void LoadDataTableToMap(UDataTable* InDataTable);
	void LoadBagData();
};
