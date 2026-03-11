// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AZLootBoxComponent.h"
#include "Item/AZItemBase.h"
#include "Item/AZBagItem.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "DataAsset/AZBagShapeDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/AZHUD.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "System/Player/AZPlayerController.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Components/CanvasPanel.h"
#include "System/AZDataManagerSubsystem.h"
#include "DataTable/AZItemDataTableRow.h"
#include "DataTable/AZDropDataTable.h"
#include "DataTable/AZBaseItemDataTable.h"

// Sets default values for this component's properties
UAZLootBoxComponent::UAZLootBoxComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}

void UAZLootBoxComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAZLootBoxComponent, bIsItemsGenerated);
}

void UAZLootBoxComponent::OnRep_Items()
{
	Super::OnRep_Items();
}

void UAZLootBoxComponent::OnRep_InventoryRevision()
{
	if (InventoryGridWidget && InventoryGridWidget->IsValidLowLevel())
	{
		InventoryGridWidget->Init(this);
		RefreshAllItems();
		InventoryGridWidget->CreateAllItemWidget(this);
	}
}


// Called when the game starts
void UAZLootBoxComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	const FBagDefinition* BagShape = DataManger->GetBagShapeByName("Chest");

	if (BagShape)
	{
		InventoryStruct = *BagShape;
	}
	Items.SetNum(InventoryStruct.Columns * InventoryStruct.Rows);
	//GenerateRandomItems();
	// ...
}


// Called every frame
void UAZLootBoxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAZLootBoxComponent::GenerateRandomItems()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
		return;

	if (!Owner->HasAuthority()) //서버만 생성
		return;
	
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	if (LootSourceType == ELootSourceType::None) //기존 테스트용 생성
	{
		TestGenerate();
	}
	else
	{
		TArray<FAZDropDataTable*> SelectedDropTable = DataManger->GetDropTable(LootSourceType);
		if (SelectedDropTable.Num() == 0)
			return;
		GenerateRandomItemsByDropTable(SelectedDropTable);
	}
	
	bAddedItem = false;
	bIsItemsGenerated = true;
}

void UAZLootBoxComponent::TestGenerate()
{
	//UGameInstance* GI = GetWorld()->GetGameInstance();
	//if (GI == nullptr)
	//	return;
	//UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	//if (DataManger == nullptr)
	//	return;

	////개수는 임시
	//int32 ItemCount = FMath::RandRange(5, 7);

	//for (int32 i = 0; i < ItemCount; i++)
	//{
	//	//int32 MaxItemID = DataManger->GetMaxItemID();
	//	int32 RandomItemID = DataManger->TestGetRandomID();
	//	if (RandomItemID == 0)
	//		continue;
	//	int32 GeneratedCount = FMath::RandRange(1, DataManger->GetMaxStackSizeByID(RandomItemID));
	//	UAZItemBase* NewItem = nullptr;
	//	EItemType FoundItemType = DataManger->GetItemTypeByID(RandomItemID);
	//	switch (FoundItemType)
	//	{
	//	case EItemType::Equipment:
	//	case EItemType::Weapon_Firearm:
	//		NewItem = NewObject<UAZItemBase>(this);
	//		break;
	//	case EItemType::Misc:
	//	case EItemType::Supplies:
	//		NewItem = NewObject<UAZItemBase>(this);
	//		break;
	//	case EItemType::Storage:
	//		NewItem = NewObject<UAZBagItem>(this);
	//		break;
	//	default:
	//		break;
	//	}

	//	if (NewItem != nullptr)
	//	{
	//		NewItem->SetItemData(RandomItemID);
	//		NewItem->SetStackCount(GeneratedCount);
	//		TryAddItem(NewItem);
	//	}
	//}
}

void UAZLootBoxComponent::AddReward(const FAZQuestReward& Reward)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
		return;

	if (!Owner->HasAuthority()) //서버만 생성
		return;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	int32 ID = DataManger->GetIDByTag(Reward.ItemTag);
	if (ID == 0) return;

	const FAZBaseItemDataTable* Data = DataManger->GetItemDataByID(ID);
	if (!Data) return;

	if (Data->MaxStack > 1)
	{
		UAZItemBase* NewItem = nullptr;
		EItemType GeneratedItemType = DataManger->GetItemTypeByID(ID);
		if (GeneratedItemType == EItemType::Storage)
			NewItem = NewObject<UAZBagItem>(this);
		else
			NewItem = NewObject<UAZItemBase>(this);

		if (NewItem != nullptr)
		{
			NewItem->SetItemData(ID);
			NewItem->SetStackCount(Reward.Amount);
		}

		if (UAZBagItem* BagItem = Cast<UAZBagItem>(NewItem))
		{
			BagItem->InitializeRandomBagData();
		}

		TryAddItem(NewItem);
	}
	else
	{
		for (int i = 0; i < Reward.Amount; ++i)
		{
			UAZItemBase* NewItem = nullptr;
			EItemType GeneratedItemType = DataManger->GetItemTypeByID(ID);
			if (GeneratedItemType == EItemType::Storage)
				NewItem = NewObject<UAZBagItem>(this);
			else
				NewItem = NewObject<UAZItemBase>(this);

			if (NewItem != nullptr)
			{
				NewItem->SetItemData(ID);
				NewItem->SetStackCount(1);
			}

			if (UAZBagItem* BagItem = Cast<UAZBagItem>(NewItem))
			{
				BagItem->InitializeRandomBagData();
			}

			TryAddItem(NewItem);
		}
	}

	
}

void UAZLootBoxComponent::GenerateRandomItemsByDropTable(TArray<FAZDropDataTable*> SelectedDropTable)
{
	if (SelectedDropTable.Num() == 0)
		return;
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	for (FAZDropDataTable* Row : SelectedDropTable)
	{
		float GeneratedChance = FMath::RandRange(1.0f, 100.0f);
		if (GeneratedChance > Row->DropChance)
			continue;
		int32 GeneratedCount = FMath::RandRange(Row->MinCount, Row->MaxCount);
		UAZItemBase* NewItem = nullptr;
		EItemType GeneratedItemType = DataManger->GetItemTypeByID(Row->ID);
		if (GeneratedItemType == EItemType::Storage)
			NewItem = NewObject<UAZBagItem>(this);
		else
			NewItem = NewObject<UAZItemBase>(this);

		if (NewItem != nullptr)
		{
			NewItem->SetItemData(Row->ID);
			NewItem->SetStackCount(GeneratedCount);
			if (UAZBagItem* BagItem = Cast<UAZBagItem>(NewItem))
			{
				BagItem->InitializeRandomBagData();
			}
			TryAddItem(NewItem);
		}
	}
}

