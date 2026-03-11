// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AZStashComponent.h"
#include "Net/UnrealNetwork.h"
#include "System/AZDataManagerSubsystem.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "DataTable/AZCraftingRecipeDataTableRow.h"
#include "Item/AZItemBase.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "Engine/GameInstance.h"
#include "System/Subsystems/UAZCurrencySubsystem.h"
#include "Item/AZBagItem.h"
#include "Components/AZBagInventoryComponent.h"
#include "Kismet/GameplayStatics.h"

UAZStashComponent::UAZStashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UAZStashComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAZStashComponent, Items);
}

void UAZStashComponent::OnRep_Items()
{
	Super::OnRep_Items();
}


void UAZStashComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAZStashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAZStashComponent::InitStashComponent()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	const FBagDefinition* BagShape = DataManger->GetBagShapeByName("Stash");

	if (BagShape)
	{
		InventoryStruct = *BagShape;
	}
	Items.SetNum(InventoryStruct.Columns * InventoryStruct.Rows);
	AAZPlayerController* AZPC = Cast<AAZPlayerController>(GetOwner());
	if (AZPC == nullptr)
		return;
	InventoryGridWidget = AZPC->InventoryWidget->StashInventoryGrid;
	if (InventoryGridWidget == nullptr)
		return;
	InventoryGridWidget->Init(this);
	if (AZPC->IsLocalController())
	{
		InventoryGridWidget->CreateInvnetorySlot();
	}
	//TestGenerateItem();
}

bool UAZStashComponent::HasIngredients(FAZCraftingRecipeDataTableRow* CraftingRecipes)
{
	if (CraftingRecipes == nullptr)
	{
		return false;
	}
	// 돈 체크
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr) return false;
	UUAZCurrencySubsystem* CurrencySubsystem = GI->GetSubsystem<UUAZCurrencySubsystem>();
	if (CurrencySubsystem == nullptr) return false;

	if (CurrencySubsystem->GetMoney() < CraftingRecipes->Cost) return false;
	
	// 아이템 수량 체크
	TMap<int32, int32> RequiredMaterials;

	auto AddRequirement = [&](int32 ID, int32 Qty)
		{
			if (ID > 0 && Qty > 0)
			{
				int32& CurrentQty = RequiredMaterials.FindOrAdd(ID);
				CurrentQty += Qty;
			}
		};
	AddRequirement(CraftingRecipes->Mat_1_ID, CraftingRecipes->Mat_1_Qty);
	AddRequirement(CraftingRecipes->Mat_2_ID, CraftingRecipes->Mat_2_Qty);
	AddRequirement(CraftingRecipes->Mat_3_ID, CraftingRecipes->Mat_3_Qty);

	if (RequiredMaterials.Num() == 0) return true;

	TArray<UAZItemBase*> ItemArray;
	GetAllItems().GetKeys(ItemArray);
	int32 TotalCount = 0;
	for (UAZItemBase* Item : ItemArray)
	{
		if (!Item) continue;

		if (int32* RequiredQtyPtr = RequiredMaterials.Find(Item->GetItemID()))
		{
			*RequiredQtyPtr -= Item->GetStackCount();
		}
	}

	for (const auto& Pair : RequiredMaterials)
	{
		if (Pair.Value > 0)
		{
			return false;
		}
	}
	return true;
}

void UAZStashComponent::RemoveItemQuantity(FAZCraftingRecipeDataTableRow* CraftingRecipes)
{
	if (CraftingRecipes == nullptr)
		return;

	TMap<int32, int32> MaterialToSupllies;

	auto SetSuppliesTarget = [&](int32 ID, int32 Qty)
		{
			if (ID > 0 && Qty > 0)
			{
				MaterialToSupllies.FindOrAdd(ID) += Qty;
			}
		};

	SetSuppliesTarget(CraftingRecipes->Mat_1_ID, CraftingRecipes->Mat_1_Qty);
	SetSuppliesTarget(CraftingRecipes->Mat_2_ID, CraftingRecipes->Mat_2_Qty);
	SetSuppliesTarget(CraftingRecipes->Mat_3_ID, CraftingRecipes->Mat_3_Qty);

	if (MaterialToSupllies.Num() == 0)
		return;

	TArray<UAZItemBase*> ItemArray;
	GetAllItems().GetKeys(ItemArray);

	for (int32 i = ItemArray.Num() - 1; i >= 0; --i)
	{
		UAZItemBase* Item = ItemArray[i];
		if (Item == nullptr) continue;

		int32 ItemID = Item->GetItemID();

		if (int32* QtyLeftPtr = MaterialToSupllies.Find(ItemID))
		{
			int32& QtyToSupplies = *QtyLeftPtr;

			// 이미 다 차감했다면 스킵 (최적화)
			if (QtyToSupplies <= 0) continue;

			int32 CurrentStack = Item->GetStackCount();

			if (CurrentStack <= QtyToSupplies)
			{
				// 아이템 전량 소모 -> 삭제
				QtyToSupplies -= CurrentStack;
				RemoveItem(Item);
			}
			else
			{
				// 아이템 일부 소모 -> 수량 변경
				Item->SetStackCount(CurrentStack - QtyToSupplies);
				QtyToSupplies = 0;
			}
		}

	}

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr) return;
	UUAZCurrencySubsystem* CurrencySubsystem = GI->GetSubsystem<UUAZCurrencySubsystem>();
	if (CurrencySubsystem == nullptr) return;

	CurrencySubsystem->SpendMoney(CraftingRecipes->Cost);
}

int32 UAZStashComponent::GetTotalItemCount(int32 MaterialID)
{
	TArray<UAZItemBase*> ItemArray;
	GetAllItems().GetKeys(ItemArray);
	int32 TotalCount = 0;
	for (UAZItemBase* Item : ItemArray)
	{
		if (Item->GetItemID() == MaterialID)
		{
			TotalCount += Item->GetStackCount();
		}
	}
	return TotalCount;
}

FItemAmountMap UAZStashComponent::GetCurrentAmounts()
{
	FItemAmountMap ItemCurrentAmounts;
	TArray<UAZItemBase*> ItemArray;
	GetAllItems().GetKeys(ItemArray);
	for (UAZItemBase* Item : ItemArray)
	{
		ItemCurrentAmounts.Map.FindOrAdd(Item->GetItemID()) += Item->GetStackCount();
	}
	return ItemCurrentAmounts;
}

void UAZStashComponent::BroadcastCurrentStashState()
{
	if (OnStashItemStateChanged.IsBound())
	{
		OnStashItemStateChanged.Broadcast(GetCurrentAmounts());
	}

}

int32 UAZStashComponent::RemoveItemByID(int32 ItemID, int32 Quantity)
{
	if (Quantity <= 0) return 0;

	int32 RemainingToRemove = Quantity;
	TArray<UAZItemBase*> ItemArray;
	GetAllItems().GetKeys(ItemArray);

	for (int32 i = 0; i < ItemArray.Num(); ++i)
	{
		UAZItemBase* Item = ItemArray[i];
		if (!Item || Item->GetItemID() != ItemID) continue;

		int32 CurrentStack = Item->GetStackCount();

		if (CurrentStack <= RemainingToRemove)
		{
			RemainingToRemove -= CurrentStack;
			RemoveItem(Item);
		}
		else
		{
			Item->SetStackCount(CurrentStack - RemainingToRemove);
			RemainingToRemove = 0;
			break;
		}

		if (RemainingToRemove <= 0) break;
	}

	return Quantity - RemainingToRemove;
}

void UAZStashComponent::TestGenerateItem()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	//if (UGameplayStatics::GetCurrentLevelName(GetWorld(), true) == TEXT("Lobbylevel_Hub"))
	//{
	//	UAZBagItem* NewBagItem = NewObject<UAZBagItem>(this);
	//	if (!NewBagItem) return;

	//	NewBagItem->SetItemID(604);
	//	NewBagItem->InitializeRandomBagData();
	//	UAZBagInventoryComponent* NewBagInventoryComp = NewObject<UAZBagInventoryComponent>(this);

	//	if (NewBagInventoryComp)
	//	{
	//		NewBagInventoryComp->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	//		NewBagInventoryComp->SetIsReplicated(true);
	//		NewBagInventoryComp->RegisterComponent();
	//		NewBagItem->InitBagItem(NewBagInventoryComp);
	//		TryAddItem(NewBagItem);
	//	}
	//}
	

	//퀘스트 아이템 테스트
	for (int32 i = 0; i < 3; i++)
	{
		UAZItemBase* Item1 = NewObject<UAZItemBase>(this);
		Item1->SetItemID(701);
		Item1->SetStackCount(5);
		TryAddItem(Item1);
	}

	for (int32 i = 0; i < 3; i++)
	{
		UAZItemBase* Item1 = NewObject<UAZItemBase>(this);
		Item1->SetItemID(702);
		Item1->SetStackCount(1);
		TryAddItem(Item1);
	}

	for (int32 i = 0; i < 3; i++)
	{
		UAZItemBase* Item1 = NewObject<UAZItemBase>(this);
		Item1->SetItemID(703);
		Item1->SetStackCount(1);
		TryAddItem(Item1);
	}

	//무기 아이템 테스트
	//UAZItemBase* Item1 = NewObject<UAZItemBase>(this);
	//Item1->SetItemID(402);
	//TryAddItem(Item1);

	//UAZItemBase* Item2 = NewObject<UAZItemBase>(this);
	//Item2->SetItemID(403);
	//TryAddItem(Item2);

	//UAZItemBase* Item3 = NewObject<UAZItemBase>(this);
	//Item3->SetItemID(404);
	//TryAddItem(Item3);

	/*for (int32 i = 0; i < 3; i++)
	{
		UAZItemBase* NewItem = NewObject<UAZItemBase>(this);
		if (NewItem != nullptr)
		{
			NewItem->SetItemData(705);
			NewItem->SetStackCount(1);
			TryAddItem(NewItem);
		}
	}
	
	for (int32 i = 0; i < 3; i++)
	{
		UAZItemBase* NewItem = NewObject<UAZItemBase>(this);
		if (NewItem != nullptr)
		{
			NewItem->SetItemData(705);
			NewItem->SetStackCount(1);
			TryAddItem(NewItem);
		}
	}

	for (int32 i = 0; i < 3; i++)
	{
		UAZItemBase* NewItem = NewObject<UAZItemBase>(this);
		if (NewItem != nullptr)
		{
			NewItem->SetItemData(101);
			NewItem->SetStackCount(8);
			TryAddItem(NewItem);
		}
	}

	for (int32 i = 0; i < 5; i++)
	{
		UAZItemBase* NewItem = NewObject<UAZItemBase>(this);
		if (NewItem != nullptr)
		{
			NewItem->SetItemData(104);
			NewItem->SetStackCount(3);
			TryAddItem(NewItem);
		}
	}*/
}
