// Fill out your copyright notice in the Description page of Project Settings.


#include "AZInventoryComponent.h"
#include "Item/AZItemBase.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "DataAsset/AZBagShapeDataAsset.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "UI/Inventory/AZItemWidget.h"
#include "Item/AZBagItem.h"
#include "Components/AZBagInventoryComponent.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AZEquipmentComponent.h"
#include "System/AZDataManagerSubsystem.h"
#include "DataTable/AZBaseItemDataTable.h"

// Sets default values for this component's properties
UAZInventoryComponent::UAZInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	static ConstructorHelpers::FClassFinder<UAZItemWidget> WidgetClassFinder(TEXT("/Game/Blueprints/UI/Inventory/WBP_Item.WBP_Item_C"));
	if (WidgetClassFinder.Succeeded())
	{
		ItemWidgetClass = WidgetClassFinder.Class;
	}
	AllItems = {};
	// ...
}


void UAZInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAZInventoryComponent, Items);
	DOREPLIFETIME(UAZInventoryComponent, InventoryStruct);
	DOREPLIFETIME(UAZInventoryComponent, ItemWidgetClass);
	DOREPLIFETIME(UAZInventoryComponent, InventoryRevision);
}

bool UAZInventoryComponent::ReplicateSubobjectsFromItems(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = false;

	// Items 배열에 들어있는 모든 유효한 아이템 객체를 복제 등록
	for (UAZItemBase* Item : Items)
	{
		if (Item && Item->IsSupportedForNetworking())
		{
			bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

// Called when the game starts
void UAZInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAZInventoryComponent::OnRep_Items()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("InventoryComponent: OnRep_Items"));
	//UE_LOG(LogTemp, Warning, TEXT("Parent OnRep Called"));
	RefreshAllItems();

	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast();
	}
	//if (InventoryGridWidget && InventoryGridWidget->IsValidLowLevel())
	//{
	//	InventoryGridWidget->Init(this);
	//	RefreshAllItems();
	//	InventoryGridWidget->CreateAllItemWidget(this);
	//}
}

void UAZInventoryComponent::OnRep_InventoryStruct()
{
	if (InventoryGridWidget && InventoryGridWidget->IsValidLowLevel())
	{
		InventoryGridWidget->CreateInvnetorySlot();
	}
}

void UAZInventoryComponent::OnRep_InventoryRevision()
{
	OnRep_Items();
}


// Called every frame
void UAZInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//if (bAddedItem)
	//{
	//	if (InventoryGridWidget != nullptr)
	//	{
	//		InventoryGridWidget->Refresh();
	//		InventoryGridWidget->CreateAllItemWidget(this);
	//		bAddedItem = false;
	//	}
	//}
}

bool UAZInventoryComponent::TryAddItem(UAZItemBase* ItemToAdd) //아이템 추가
{
	if (ItemToAdd)
	{
		UE_LOG(LogTemp, Warning, TEXT("Items.Num(): %d"), Items.Num());
		for (int32 i = 0; i < Items.Num(); i++)	// 아이템 Array를 전부 순회
		{
			if (IsRoomAvailable(ItemToAdd, i))	// 아이템이 들어갈 수 있는지 확인
			{
				AddItemAt(ItemToAdd, i);		// 그 위치에 아이템을 입력
				return true;
			}
			ItemToAdd->RotateItem();			// 회전해서 아이템을 넣을 수 있는지 확인
			if (IsRoomAvailable(ItemToAdd, i))
			{
				AddItemAt(ItemToAdd, i);
				return true;
			}
			ItemToAdd->RotateItem();
		}
		
		return false;
	}
	return false;
}

bool UAZInventoryComponent::IsRoomAvailable(UAZItemBase* ItemToAdd, int32 TopLeftIndex)
{
	FIntPoint Dimensions = ItemToAdd->GetDimensions();
	FIntPoint Tile = IndexToTile(TopLeftIndex);

	for (int32 i = Tile.X; i <= Tile.X + Dimensions.X - 1; i++)
	{
		for (int32 j = Tile.Y; j <= Tile.Y + Dimensions.Y - 1; j++)
		{
			if (IsValidTile(FIntPoint(i,j)))
			{
				int32 Index = TileToIndex(FIntPoint(i, j));
				if (!IsVaildIndex(Index))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}

FIntPoint UAZInventoryComponent::IndexToTile(int32 Index)
{
	return FIntPoint(Index % InventoryStruct.Columns, Index / InventoryStruct.Columns);
}

bool UAZInventoryComponent::IsValidTile(FIntPoint Tile)
{
	if ((Tile.X >= 0 && Tile.Y >= 0 && Tile.X < InventoryStruct.Columns && Tile.Y < InventoryStruct.Rows) 
		&& InventoryStruct.GridFlags[Tile.Y * InventoryStruct.Columns + Tile.X] == true)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

int32 UAZInventoryComponent::TileToIndex(FIntPoint Tile)
{
	int32 Index = Tile.X + Tile.Y * InventoryStruct.Columns;
	return Index;
}

bool UAZInventoryComponent::IsVaildIndex(int32 Index)
{
	if (Items.IsValidIndex(Index))
	{
		UAZItemBase* Item = GetItemAtIndex(Index);
		if (Item == nullptr)
			return true;
		if (Item->GetIsDragged())
			return true;
	}
	return false;
}
bool UAZInventoryComponent::GetResultAtIndex(int32 Index)
{
	if (Items.IsValidIndex(Index))
	{
		return true;
	}
	return false;
}

UAZItemBase* UAZInventoryComponent::GetItemAtIndex(int32 Index)
{
	if (Items.IsValidIndex(Index))
	{
		return Items[Index];
	}
	return nullptr;
}

int32 UAZInventoryComponent::GetIndexAtItem(UAZItemBase* Item)
{
	int32 Index = Items.Find(Item);
	return Index;
}

void UAZInventoryComponent::AddItemAt(UAZItemBase* ItemToAdd, int32 TopLeftIndex)
{
	if (ItemToAdd == nullptr)
		return;
	if (!Items.IsValidIndex(TopLeftIndex))
		return;
	UE_LOG(LogTemp, Display, TEXT("AddItem"));
	FIntPoint Dimensions = ItemToAdd->GetDimensions();
	FIntPoint Tile = IndexToTile(TopLeftIndex);
	ItemToAdd->SetIsDragged(false);
	for (int32 i = Tile.X; i <= Tile.X + Dimensions.X - 1; i++)
	{
		for (int32 j = Tile.Y; j <= Tile.Y + Dimensions.Y - 1; j++)
		{
			Items[TileToIndex(FIntPoint(i, j))] = ItemToAdd;
		}
	}
	//bAddedItem = true;
	ItemToAdded = ItemToAdd;
	if (InventoryGridWidget != nullptr)
	{
		InventoryGridWidget->bIsDropped = false;
	}
}

void UAZInventoryComponent::AddItemAt_Server_Implementation(UAZItemBase* ItemToAdd, int32 TopLeftIndex)
{
	if (ItemToAdd == nullptr)
		return;
	
	if (ItemToAdd->GetOuter() != this)
	{
		ItemToAdd->Rename(nullptr, this);
	}

	UE_LOG(LogTemp, Display, TEXT("AddItem"));
	FIntPoint Dimensions = ItemToAdd->GetDimensions();
	FIntPoint Tile = IndexToTile(TopLeftIndex);

	for (int32 i = Tile.X; i <= Tile.X + Dimensions.X - 1; i++)
	{
		for (int32 j = Tile.Y; j <= Tile.Y + Dimensions.Y - 1; j++)
		{
			Items[TileToIndex(FIntPoint(i, j))] = ItemToAdd;
		}
	}
	bAddedItem = true;
	ItemToAdded = ItemToAdd;
	if (InventoryGridWidget != nullptr)
	{
		InventoryGridWidget->bIsDropped = false;
	}
}

TMap<class UAZItemBase*, FIntPoint> UAZInventoryComponent::GetAllItems()
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i])
		{
			if (!AllItems.Contains(Items[i]))
			{
				AllItems.Add(Items[i], IndexToTile(i));
			}
		}
	}
	return AllItems;
}

void UAZInventoryComponent::SetInventoryGridWidget(UAZInventoryGridWidget* GridWidget)
{
	InventoryGridWidget = GridWidget;
}

void UAZInventoryComponent::RemoveItem(UAZItemBase* ItemToRemove)
{
	if (ItemToRemove)
	{
		for (int32 i = 0; i < Items.Num(); i++)
		{
			if (Items[i] == ItemToRemove)
			{
				Items[i] = nullptr;
			}
		}
		if (AllItems.Contains(ItemToRemove))
		{
			AllItems.Remove(ItemToRemove);
		}
	}

	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast();
	}
}

void UAZInventoryComponent::RemoveItem_Server_Implementation(UAZItemBase* ItemToRemove)
{
	if (ItemToRemove)
	{
		for (int32 i = 0; i < Items.Num(); i++)
		{
			if (Items[i] == ItemToRemove)
			{
				Items[i] = nullptr;
			}
		}
		if (AllItems.Contains(ItemToRemove))
		{
			AllItems.Remove(ItemToRemove);
		}
	}
}

void UAZInventoryComponent::RefreshAllItems()
{
	AllItems.Empty();

	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i] != nullptr)
		{
			if (!AllItems.Contains(Items[i]))
			{
				AllItems.Add(Items[i], IndexToTile(i));
			}
		}
	}
}

int32 UAZInventoryComponent::RemoveItemByTag(FGameplayTag GamPlayTag, int32 Amount)
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr) return 0;
	UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManager == nullptr) return 0 ;

	if (AllItems.Num() == 0)
	{
		return Amount;
	}
	int32 RemainingAmountToRemove = Amount;
	RefreshAllItems();

	TArray<UAZItemBase*> ItemsToCheck;
	AllItems.GetKeys(ItemsToCheck);
	


	for (UAZItemBase* Item : ItemsToCheck)
	{
		if (Item == nullptr) continue;
		const FAZBaseItemDataTable* ItemData = DataManager->GetItemDataByID(Item->GetItemID());
		if (ItemData == nullptr) continue;
		if (ItemData->GamePlayTag.MatchesTag(GamPlayTag))
		{
			int32 CurrentStack = Item->GetStackCount();
			if (CurrentStack > RemainingAmountToRemove)
			{
				int32 NewStack = CurrentStack - RemainingAmountToRemove;
				SetItemStack_Multicast(Item, NewStack);
				RemainingAmountToRemove = 0;
				break;
			}
			else
			{
				RemainingAmountToRemove -= CurrentStack;
				RemoveItem_Server(Item);
			}
		}

		if (RemainingAmountToRemove <= 0)
		{
			break;
		}
	}
	OnRep_Items();
	return Amount - RemainingAmountToRemove;
}

void UAZInventoryComponent::SetDraggedItemLastIndex(int32 Index)
{
	DraggedItemLastIndex = Index;
}

int32 UAZInventoryComponent::GetDraggedItemLastIndex()
{
	return DraggedItemLastIndex;
}

FBagDefinition UAZInventoryComponent::GetInventoryStruct()
{
	return InventoryStruct;
}


void UAZInventoryComponent::InventoryRefresh()
{
	if (InventoryGridWidget && InventoryGridWidget->IsValidLowLevel())
	{
		RefreshAllItems();
		InventoryGridWidget->CreateAllItemWidget(this);
	}
}

void UAZInventoryComponent::ResetInventoryData_Server_Implementation()
{
	//InventoryStruct = FBagDefinition();
}


void UAZInventoryComponent::SetItemStack_Multicast_Implementation(UAZItemBase* Item, int32 Count)
{
	if (Item == nullptr)
		return;
	Item->SetStackCount(Count);
	OnRep_Items();
}
