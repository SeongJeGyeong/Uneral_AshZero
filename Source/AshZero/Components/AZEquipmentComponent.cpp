// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AZEquipmentComponent.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Inventory/AZEquipmentSlot.h"
#include "UI/Operation/AZDragDropOperation.h"
#include "UI/HUD/AZQuickSlotWidget.h"
#include "Components/AZItemUsageComponent.h"
#include "Components/AZPlayerInventoryComponent.h"
#include "Components/VerticalBox.h"
#include "System/AZDataManagerSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "Item/AZItemBase.h"
#include "Components/TextBlock.h"
#include "Character/AZPlayerCharacter.h"
#include "UI/HUD/AZStatusBarWidget.h"
#include "Net/UnrealNetwork.h"
#include "UI/Inventory/AZItemWidget.h"
#include "AZStatusTypes.h"
#include "DataTable/AZEquipmentItemDataTableRow.h"
#include "Components/AZHealthComponent.h"
#include "Components/AZPlayerMoveComponent.h"
#include "Item/AZBagItem.h"
#include "DataTable/AZStorageItemDataTableRow.h"
#include "Components/AZBagInventoryComponent.h"
#include "Components/AZPlayerFireComponent.h"
#include "DataTable/AZWeaponItemDataTableRow.h"
#include "Weapon/AZWeapon.h"
#include "Engine/ActorChannel.h"
#include "System/Subsystems/AZSceneSubsystem.h"

// Sets default values for this component's properties
UAZEquipmentComponent::UAZEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}

void UAZEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAZEquipmentComponent, Helmet);
	DOREPLIFETIME(UAZEquipmentComponent, Armor);
	DOREPLIFETIME(UAZEquipmentComponent, Gloves);
	DOREPLIFETIME(UAZEquipmentComponent, Boots);
	DOREPLIFETIME(UAZEquipmentComponent, FirstWeapon);
	DOREPLIFETIME(UAZEquipmentComponent, SecondWeapon);
	DOREPLIFETIME(UAZEquipmentComponent, Backpack);
	DOREPLIFETIME(UAZEquipmentComponent, QuickSlotItem_1);
	DOREPLIFETIME(UAZEquipmentComponent, QuickSlotItem_2);
	DOREPLIFETIME(UAZEquipmentComponent, QuickSlotItem_3);
	DOREPLIFETIME(UAZEquipmentComponent, QuickSlotItem_4);
}

bool UAZEquipmentComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = false;

	if (Helmet) bWroteSomething |= Channel->ReplicateSubobject(Helmet, *Bunch, *RepFlags);
	if (Armor) bWroteSomething |= Channel->ReplicateSubobject(Armor, *Bunch, *RepFlags);
	if (Gloves) bWroteSomething |= Channel->ReplicateSubobject(Gloves, *Bunch, *RepFlags);
	if (Boots) bWroteSomething |= Channel->ReplicateSubobject(Boots, *Bunch, *RepFlags);
	if (Backpack) bWroteSomething |= Channel->ReplicateSubobject(Backpack, *Bunch, *RepFlags);

	if (FirstWeapon) bWroteSomething |= Channel->ReplicateSubobject(FirstWeapon, *Bunch, *RepFlags);
	if (SecondWeapon) bWroteSomething |= Channel->ReplicateSubobject(SecondWeapon, *Bunch, *RepFlags);

	if (QuickSlotItem_1) bWroteSomething |= Channel->ReplicateSubobject(QuickSlotItem_1, *Bunch, *RepFlags);
	if (QuickSlotItem_2) bWroteSomething |= Channel->ReplicateSubobject(QuickSlotItem_2, *Bunch, *RepFlags);
	if (QuickSlotItem_3) bWroteSomething |= Channel->ReplicateSubobject(QuickSlotItem_3, *Bunch, *RepFlags);
	if (QuickSlotItem_4) bWroteSomething |= Channel->ReplicateSubobject(QuickSlotItem_4, *Bunch, *RepFlags);

	return bWroteSomething;
}
// Called when the game starts
void UAZEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}
void UAZEquipmentComponent::SetUsedQuickSlotIndex(ESlotIndex NewUsedQuickSlotIndex)
{
	UsedQuickSlotIndex = NewUsedQuickSlotIndex;
}

void UAZEquipmentComponent::SubUsedQuickSlotItem()
{
	if (UsedQuickSlotIndex == ESlotIndex::Slot_0) return;

	SubQuickSlotItem(UsedQuickSlotIndex);
}

TObjectPtr<UAZItemBase>& UAZEquipmentComponent::GetItemRefBySlot(EEquipmentSlot SlotType, ESlotIndex SlotIndex)
{
	switch (SlotType)
	{
	case EEquipmentSlot::Helmet:	return Helmet;
	case EEquipmentSlot::Armor:		return Armor;
	case EEquipmentSlot::Gloves:	return Gloves;
	case EEquipmentSlot::Boots:		return Boots;
	case EEquipmentSlot::Backpack:	return Backpack;
	case EEquipmentSlot::Weapon:
		return (SlotIndex == ESlotIndex::Slot_0) ? FirstWeapon: SecondWeapon;
	case EEquipmentSlot::QuickSlot:
		switch (SlotIndex)
		{
		case ESlotIndex::Slot_1: return QuickSlotItem_1;
		case ESlotIndex::Slot_2: return QuickSlotItem_2;
		case ESlotIndex::Slot_3: return QuickSlotItem_3;
		case ESlotIndex::Slot_4: return QuickSlotItem_4;
		default: break;
		}
		break;
	}
	static TObjectPtr<UAZItemBase> Dummy = nullptr;
	return Dummy;
}


void UAZEquipmentComponent::UpdateReplicatedStats_Server_Implementation()
{

	AAZPlayerController* PlayerController = Cast<AAZPlayerController>(GetOwner());
	if (PlayerController == nullptr)
		return;
	AAZPlayerCharacter* PlayerCharacter = Cast<AAZPlayerCharacter>(PlayerController->GetPawn());
	if (PlayerCharacter == nullptr)
		return;
	UAZHealthComponent* HealthComponent = PlayerCharacter->GetComponentByClass<UAZHealthComponent>();
	if (HealthComponent == nullptr)
		return;
	UAZPlayerMoveComponent* PlayerMoveComponent = PlayerCharacter->GetComponentByClass<UAZPlayerMoveComponent>();
	if (PlayerMoveComponent == nullptr)
		return;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	TArray<UAZItemBase*> Items = { Helmet, Armor, Gloves, Boots};
	float NewMaxHP = HealthComponent->BaseMaxHp;
	float SpeedPercentBonus = 0.0f;
	for (UAZItemBase* Item : Items)
	{
		if (Item == nullptr)
			continue;
		const FAZEquipmentItemDataTableRow* Data = DataManger->GetItemDataByID<FAZEquipmentItemDataTableRow>(Item->GetItemID());
		if (Data == nullptr)
			continue;
		NewMaxHP += Data->Gives_Benefit_HP;
		SpeedPercentBonus += Data->Gives_Benefit_Speed;
	}

	HealthComponent->MaxHp = NewMaxHP;
	HealthComponent->OnRep_Hp();
	if (Backpack == nullptr)
		return;
	const FAZStorageItemDataTableRow* StorageData = DataManger->GetItemDataByID<FAZStorageItemDataTableRow>(Backpack->GetItemID());
	if (StorageData == nullptr)
		return;
	PlayerMoveComponent->UpdateSpeed_Server(SpeedPercentBonus, StorageData->MoveSpeed_Penalty);
}

void UAZEquipmentComponent::UpdateLocalStats()
{
	AAZPlayerController* PlayerController = Cast<AAZPlayerController>(GetOwner());
	if (PlayerController == nullptr)
		return;
	AAZPlayerCharacter* PlayerCharacter = Cast<AAZPlayerCharacter>(PlayerController->GetPawn());
	if (PlayerCharacter == nullptr)
		return;
	UAZPlayerMoveComponent* PlayerMoveComponent = PlayerCharacter->GetComponentByClass<UAZPlayerMoveComponent>();
	if (PlayerMoveComponent == nullptr)
		return;
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	TArray<UAZItemBase*> Items = { Helmet, Armor, Gloves, Boots };
	float NewMaxStamina = PlayerMoveComponent->BaseStamina;
	for (UAZItemBase* Item : Items)
	{
		if (Item == nullptr)
			continue;
		const FAZEquipmentItemDataTableRow* Data = DataManger->GetItemDataByID<FAZEquipmentItemDataTableRow>(Item->GetItemID());
		if (Data == nullptr)
			continue;
		NewMaxStamina += Data->Gives_Benefit_HP;
	}
	PlayerMoveComponent->MaxStamina = NewMaxStamina;
	PlayerMoveComponent->OnStaminaChanged.Broadcast(PlayerMoveComponent->CurrentStamina, PlayerMoveComponent->MaxStamina);
}

void UAZEquipmentComponent::EquipBagItem(UAZBagItem* NewBagItem, UAZInventoryComponent* SourceInventory)
{
	EquipBagItem_Server(NewBagItem, SourceInventory);

	AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
	if (!PC) return;

	if (PC->InventoryWidget && PC->InventoryWidget->BagInventoryGrid)
	{
		PC->InventoryWidget->SelectedInventoryVerticalBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAZEquipmentComponent::EquipBagItem_Server_Implementation(UAZBagItem* NewBagItem, UAZInventoryComponent* SourceInventory)
{
	AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
	if (!PC) return;
	UAZPlayerInventoryComponent* PlayerInventory = PC->InventoryComp;
	if (!PlayerInventory) return;

	UAZBagItem* OldBagItem = Cast<UAZBagItem>(Backpack);

	int32 SourceItemIndex = SourceInventory->GetIndexAtItem(NewBagItem);
	if (SourceItemIndex == -1) return;

	SourceInventory->RemoveItem(NewBagItem);

	if (OldBagItem)
	{
		if (OldBagItem->InventoryComponent == nullptr)
		{
			OldBagItem->InventoryComponent = NewObject<UAZBagInventoryComponent>(OldBagItem);
			OldBagItem->InventoryComponent->SetIsReplicated(true);
		}
		OldBagItem->InventoryComponent->InventoryStruct = PlayerInventory->InventoryStruct;
		OldBagItem->InventoryComponent->Items = PlayerInventory->Items;

		for (UAZItemBase* Item : OldBagItem->InventoryComponent->Items)
		{
			if (Item)
			{
				Item->Rename(nullptr, OldBagItem->InventoryComponent);
			}
		}
	}

	if (NewBagItem && NewBagItem->InventoryComponent)
	{
		PlayerInventory->InventoryStruct = NewBagItem->InventoryComponent->InventoryStruct;
		PlayerInventory->Items = NewBagItem->InventoryComponent->Items;

		for (UAZItemBase* Item : PlayerInventory->Items)
		{
			if (Item)
			{
				Item->Rename(nullptr, PlayerInventory);
			}
		}
	}

	if (OldBagItem)
	{
		OldBagItem->Rename(nullptr, SourceInventory);
		//SourceInventory->Items[SourceItemIndex] = OldBagItem;
		SourceInventory->AddItemAt(OldBagItem, SourceItemIndex);
	}

	NewBagItem->Rename(nullptr, this);
	Backpack = NewBagItem;

	PlayerInventory->OnRep_Items();
	PlayerInventory->OnRep_InventoryStruct();

	if (SourceInventory)
	{
		SourceInventory->OnRep_Items();
	}

	HandleItemRepNotify(EEquipmentSlot::Backpack, ESlotIndex::Slot_0, NewBagItem);
}

void UAZEquipmentComponent::RefreshSourceInventory_Client_Implementation(UAZInventoryComponent* SourceInventory)
{
	if (SourceInventory)
	{
		SourceInventory->OnRep_Items();
	}
}

void UAZEquipmentComponent::RefreshSlots()
{
	TArray<UAZItemBase*> EquipmentItems = { Helmet, Armor, Gloves, Boots, Backpack };
	for (UAZItemBase* Item : EquipmentItems)
	{
		if (Item == nullptr) continue;
		HandleItemRepNotify(Item->GetEquipmentSlotType(), ESlotIndex::Slot_0, Item);
	}
	HandleItemRepNotify(EEquipmentSlot::Weapon, ESlotIndex::Slot_0, FirstWeapon);
	HandleItemRepNotify(EEquipmentSlot::Weapon, ESlotIndex::Slot_1, SecondWeapon);

	TArray<UAZItemBase*> QuickSlotItems = { QuickSlotItem_1, QuickSlotItem_2, QuickSlotItem_3, QuickSlotItem_4 };
	int32 QuickSlotIndex = 0;
	for (UAZItemBase* Item : QuickSlotItems)
	{
		if (Item == nullptr) continue;
		ESlotIndex SlotIndex = ESlotIndex(++QuickSlotIndex);
		HandleItemRepNotify(EEquipmentSlot::QuickSlot, SlotIndex, Item);
	}
}

// Called every frame
void UAZEquipmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAZEquipmentComponent::EquipItem(EEquipmentSlot SlotType, UAZDragDropOperation* AZInOperation)
{
	UAZItemWidget* ItemWidget = Cast<UAZItemWidget>(AZInOperation->DefaultDragVisual);
	if (ItemWidget == nullptr)
		return;
	UAZItemBase* Item = Cast<UAZItemBase>(AZInOperation->Payload);
	if (Item == nullptr)
		return;
	int32 ItemLastIndex = ItemWidget->InventoryComponent->GetDraggedItemLastIndex();


	EquipItem_Server(SlotType, Item, ItemWidget->InventoryComponent, ItemLastIndex, AZInOperation->ItemSourceType, AZInOperation->SlotIndex, AZInOperation->SourceEquipmentSlot);

}

void UAZEquipmentComponent::EquipItem_Server_Implementation(EEquipmentSlot SlotType, UAZItemBase* NewItem, UAZInventoryComponent* InventoryComponent, int32 ItemLastIndex, EItemSourceType ItemSourceType, ESlotIndex SlotIndex, UAZEquipmentSlot* SourceEquipmentSlot)
{
	TObjectPtr<UAZItemBase>& TargetSlotItemRef = GetItemRefBySlot(SlotType, SlotIndex);
	if (TargetSlotItemRef != nullptr && TargetSlotItemRef == NewItem)
		return;
	if (ItemSourceType != EItemSourceType::Inventory)	//드래그 드랍한 아이템이 인벤토리에 있던 아이템이 아니면
	{
		if (SlotType == EEquipmentSlot::Weapon)
		{
			if ((ItemSourceType == EItemSourceType::FirstWeapon && SlotIndex == ESlotIndex::Slot_1) || (ItemSourceType == EItemSourceType::SecondWeapon && SlotIndex == ESlotIndex::Slot_0))
			{
				SwapWeaponItem();
				return;
			}
		}
		else if ((ItemSourceType == EItemSourceType::QuickSlot_1 && SlotIndex != ESlotIndex::Slot_1) ||
			(ItemSourceType == EItemSourceType::QuickSlot_2 && SlotIndex != ESlotIndex::Slot_2) ||
			(ItemSourceType == EItemSourceType::QuickSlot_3 && SlotIndex != ESlotIndex::Slot_3) ||
			(ItemSourceType == EItemSourceType::QuickSlot_4 && SlotIndex != ESlotIndex::Slot_4))
		{
			TObjectPtr<UAZItemBase>& DestItemRef = GetQuickSlotItemRef(SlotIndex);
			TObjectPtr<UAZItemBase>& SourceItemRef = GetQuickSlotItemRef(ItemSourceType);
			if (DestItemRef != nullptr && SourceItemRef != nullptr && DestItemRef->GetItemID() == SourceItemRef->GetItemID())
				TryStackQuickSlotItem(ItemSourceType, SlotIndex);
			else
				SwapQuickSlotItem(ItemSourceType, SlotIndex);
			return;
		}
	}

	UAZItemBase* OldItem = TargetSlotItemRef;
	bool bEquipSuccess = false;
	if (OldItem != nullptr)
	{
		if (OldItem->bIsStarterItem)
		{
			NewItem->SetIsDragged(false);
			return;
		}
		/*if ((*SelectedSlotItem)->GetItemID() == NewItem->GetItemID())
		{
			UGameInstance* GI = GetWorld()->GetGameInstance();
			if (GI == nullptr)
				return;
			UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
			if (DataManger == nullptr)
				return;
			if (DataManger->GetMaxStackSizeByID(NewItem->GetItemID()) > 1)
			{

			}
		}*/
		if (InventoryComponent->IsRoomAvailable(OldItem, ItemLastIndex))
		{
			OldItem->SetIsRotated(false);
			InventoryComponent->RemoveItem(NewItem);
			InventoryComponent->AddItemAt(OldItem, ItemLastIndex);
			TargetSlotItemRef = NewItem;
			bEquipSuccess = true;
		}
		else if(InventoryComponent->TryAddItem(OldItem))
		{
			OldItem->SetIsRotated(false);
			InventoryComponent->RemoveItem(NewItem);
			TargetSlotItemRef = NewItem;
			bEquipSuccess = true;
		}
	}
	else
	{
		InventoryComponent->RemoveItem(NewItem);
		TargetSlotItemRef = NewItem;
		bEquipSuccess = true;
	}

	if (bEquipSuccess)
	{
		if (SlotType == EEquipmentSlot::Weapon) //무기를 장착 시
		{
			AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
			if (PC == nullptr) return;
			AAZPlayerCharacter* Character = Cast<AAZPlayerCharacter>(PC->GetPawn());
			if (Character == nullptr) return;

			UAZPlayerFireComponent* FireComponent = Character->FireComp;
			if (FireComponent == nullptr) return;

			UGameInstance* GI = GetWorld()->GetGameInstance();
			if (GI == nullptr) return;
			UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
			if (DataManger == nullptr) return;

			const FAZWeaponItemDataTableRow* WeaponData = DataManger->GetItemDataByID<FAZWeaponItemDataTableRow>(NewItem->GetItemID());

			if (WeaponData && WeaponData->WeaponActorClass)
			{
				int32 WeaponSlotIndex = int32(SlotIndex);
				FireComponent->EquipWeaponToSlot(WeaponData->WeaponActorClass, WeaponSlotIndex, WeaponData->Level);
			}
		}

		if (ItemSourceType == EItemSourceType::Inventory)
		{
			InventoryComponent->InventoryRefresh();
		}
		HandleItemRepNotify(SlotType, SlotIndex, TargetSlotItemRef);
	}

}

void UAZEquipmentComponent::HandleItemRepNotify(EEquipmentSlot SlotType, ESlotIndex SlotIndex, UAZItemBase* Item)
{
	if (SlotType != EEquipmentSlot::QuickSlot)
	{
		UpdateReplicatedStats_Server();
		UpdateLocalStats();
	}

	if (OnEquipmentUpdated.IsBound())
	{
		OnEquipmentUpdated.Broadcast(SlotType, SlotIndex, Item);
	}


}

void UAZEquipmentComponent::UnEquipItem(EEquipmentSlot SlotType, ESlotIndex SlotIndex)
{
	UnEquipItem_Server(SlotType, SlotIndex);
}

void UAZEquipmentComponent::SwapWeaponItem()
{
	if (FirstWeapon->bIsStarterItem || SecondWeapon->bIsStarterItem)
		return;
	Swap(FirstWeapon, SecondWeapon);
	OnRep_FirstWeapon();
	OnRep_SecondWeapon();
}

void UAZEquipmentComponent::TryStackQuickSlotItem(EItemSourceType ItemSourceType, ESlotIndex SlotIndex)
{
	TObjectPtr<UAZItemBase>& DestItemRef = GetQuickSlotItemRef(SlotIndex);
	TObjectPtr<UAZItemBase>& SourceItemRef = GetQuickSlotItemRef(ItemSourceType);

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	int32 ItemStackcount = SourceItemRef->GetStackCount();
	int32 DestItemStackCount = DestItemRef->GetStackCount();
	int32 SumCount = ItemStackcount + DestItemStackCount;
	int32 MaxStackCount = DataManger->GetMaxStackSizeByID(SourceItemRef->GetItemID());


	if (SumCount > MaxStackCount)
	{
		SetStackQuickSlotItemsCount_NetMulticast(ItemSourceType, SlotIndex, SumCount - MaxStackCount, MaxStackCount);
	}
	else
	{
		SourceItemRef = nullptr;
		SetStackQuickSlotItemsCount_NetMulticast(ItemSourceType, SlotIndex, 0, SumCount);
	}

}

void UAZEquipmentComponent::SetStackQuickSlotItemsCount_NetMulticast_Implementation(EItemSourceType ItemSourceType, ESlotIndex SlotIndex, int32 SourceCount, int32 DestCount)
{
	TObjectPtr<UAZItemBase>& DestItemRef = GetQuickSlotItemRef(SlotIndex);
	TObjectPtr<UAZItemBase>& SourceItemRef = GetQuickSlotItemRef(ItemSourceType);
	if (DestItemRef)
		DestItemRef->SetStackCount(DestCount);
	if (SourceItemRef)
		SourceItemRef->SetStackCount(SourceCount);

	switch (SlotIndex)
	{
	case ESlotIndex::Slot_1: OnRep_QuickSlot_1(); break;
	case ESlotIndex::Slot_2: OnRep_QuickSlot_2(); break;
	case ESlotIndex::Slot_3: OnRep_QuickSlot_3(); break;
	case ESlotIndex::Slot_4: OnRep_QuickSlot_4(); break;
	default: break;
	}

	switch (ItemSourceType)
	{
	case EItemSourceType::QuickSlot_1: OnRep_QuickSlot_1(); break;
	case EItemSourceType::QuickSlot_2: OnRep_QuickSlot_2(); break;
	case EItemSourceType::QuickSlot_3: OnRep_QuickSlot_3(); break;
	case EItemSourceType::QuickSlot_4: OnRep_QuickSlot_4(); break;
	default: break;
	}
}

void UAZEquipmentComponent::SwapQuickSlotItem(EItemSourceType ItemSourceType, ESlotIndex SlotIndex)
{
	TObjectPtr<UAZItemBase>& DestItemRef = GetQuickSlotItemRef(SlotIndex);
	TObjectPtr<UAZItemBase>& SourceItemRef = GetQuickSlotItemRef(ItemSourceType);
	Swap(DestItemRef, SourceItemRef);

	switch (SlotIndex)
	{
	case ESlotIndex::Slot_1: OnRep_QuickSlot_1(); break;
	case ESlotIndex::Slot_2: OnRep_QuickSlot_2(); break;
	case ESlotIndex::Slot_3: OnRep_QuickSlot_3(); break;
	case ESlotIndex::Slot_4: OnRep_QuickSlot_4(); break;
	default: break;
	}

	switch (ItemSourceType)
	{
	case EItemSourceType::QuickSlot_1: OnRep_QuickSlot_1(); break;
	case EItemSourceType::QuickSlot_2: OnRep_QuickSlot_2(); break;
	case EItemSourceType::QuickSlot_3: OnRep_QuickSlot_3(); break;
	case EItemSourceType::QuickSlot_4: OnRep_QuickSlot_4(); break;
	default: break;
	}


}

void UAZEquipmentComponent::UnEquipItem_Server_Implementation(EEquipmentSlot SlotType, ESlotIndex SlotIndex)
{
	TObjectPtr<UAZItemBase>& TargetSlotItemRef = GetItemRefBySlot(SlotType, SlotIndex);
	if (TargetSlotItemRef == nullptr)
		return;
	if (TargetSlotItemRef->bIsStarterItem)
	{
		return;
	}
	TargetSlotItemRef = nullptr;

	if (SlotType == EEquipmentSlot::Weapon)
	{
		AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
		if (PC && PC->GetPawn())
		{
			UAZPlayerFireComponent* FireComp = PC->GetPawn()->FindComponentByClass<UAZPlayerFireComponent>();
			if (FireComp)
			{
				int32 FireCompSlotIndex = int32(SlotIndex);

				FireComp->UnEquipWeaponFromSlot(FireCompSlotIndex);
			}
		}
	}
	HandleItemRepNotify(SlotType, SlotIndex, TargetSlotItemRef);
}

TObjectPtr<UAZItemBase>& UAZEquipmentComponent::GetQuickSlotItemRef(ESlotIndex SlotIndex)
{
	switch (SlotIndex)
	{
	case ESlotIndex::Slot_1: return QuickSlotItem_1;
	case ESlotIndex::Slot_2: return QuickSlotItem_2;
	case ESlotIndex::Slot_3: return QuickSlotItem_3;
	case ESlotIndex::Slot_4: return QuickSlotItem_4;
	default:
		static TObjectPtr<UAZItemBase> Dummy = nullptr;
		return Dummy;
	}
}

TObjectPtr<UAZItemBase>& UAZEquipmentComponent::GetQuickSlotItemRef(EItemSourceType ItemSourceType)
{
	switch (ItemSourceType)
	{
	case EItemSourceType::QuickSlot_1:	return QuickSlotItem_1;
	case EItemSourceType::QuickSlot_2:	return QuickSlotItem_2;
	case EItemSourceType::QuickSlot_3:	return QuickSlotItem_3;
	case EItemSourceType::QuickSlot_4:	return QuickSlotItem_4;
	default:
		static TObjectPtr<UAZItemBase> Dummy = nullptr;
		return Dummy;
	}
}

void UAZEquipmentComponent::SetQuickSlotStackCount_Multicast_Implementation(ESlotIndex SlotIndex, int32 Count)
{
	TObjectPtr<UAZItemBase>& QuickSlotItemRef = GetQuickSlotItemRef(SlotIndex);
	QuickSlotItemRef->SetStackCount(Count);

	switch (SlotIndex)
	{
	case ESlotIndex::Slot_1: OnRep_QuickSlot_1(); break;
	case ESlotIndex::Slot_2: OnRep_QuickSlot_2(); break;
	case ESlotIndex::Slot_3: OnRep_QuickSlot_3(); break;
	case ESlotIndex::Slot_4: OnRep_QuickSlot_4(); break;
	default: break;
	}
}

void UAZEquipmentComponent::SubQuickSlotItem(ESlotIndex SlotIndex)
{
	SubQuickSlotItem_Server(SlotIndex);
}

void UAZEquipmentComponent::SubQuickSlotItem_Server_Implementation(ESlotIndex SlotIndex)
{
	TObjectPtr<UAZItemBase>& QuickSlotItemRef = GetQuickSlotItemRef(SlotIndex);
	int32 Count = QuickSlotItemRef->GetStackCount();
	if (--Count == 0)
		UnEquipItem(EEquipmentSlot::QuickSlot, SlotIndex);
	else
	{
		QuickSlotItemRef->SetStackCount(Count);
		SetQuickSlotStackCount_Multicast(SlotIndex, Count);
	}
}

void UAZEquipmentComponent::OnRep_Helmet()
{
	HandleItemRepNotify(EEquipmentSlot::Helmet, ESlotIndex::Slot_0, Helmet);
}

void UAZEquipmentComponent::OnRep_Armor()
{
	HandleItemRepNotify(EEquipmentSlot::Armor, ESlotIndex::Slot_0, Armor);
}

void UAZEquipmentComponent::OnRep_Gloves()
{
	HandleItemRepNotify(EEquipmentSlot::Gloves, ESlotIndex::Slot_0, Gloves);
}

void UAZEquipmentComponent::OnRep_Boots()
{
	HandleItemRepNotify(EEquipmentSlot::Boots, ESlotIndex::Slot_0, Boots);
}

void UAZEquipmentComponent::OnRep_Backpack()
{
	HandleItemRepNotify(EEquipmentSlot::Backpack, ESlotIndex::Slot_0, Backpack);
}

void UAZEquipmentComponent::OnRep_FirstWeapon()
{
	HandleItemRepNotify(EEquipmentSlot::Weapon, ESlotIndex::Slot_0, FirstWeapon);
}

void UAZEquipmentComponent::OnRep_SecondWeapon()
{
	HandleItemRepNotify(EEquipmentSlot::Weapon, ESlotIndex::Slot_1, SecondWeapon);
}

void UAZEquipmentComponent::OnRep_QuickSlot_1()
{
	HandleItemRepNotify(EEquipmentSlot::QuickSlot, ESlotIndex::Slot_1, QuickSlotItem_1);
}

void UAZEquipmentComponent::OnRep_QuickSlot_2()
{
	HandleItemRepNotify(EEquipmentSlot::QuickSlot, ESlotIndex::Slot_2, QuickSlotItem_2);
}

void UAZEquipmentComponent::OnRep_QuickSlot_3()
{
	HandleItemRepNotify(EEquipmentSlot::QuickSlot, ESlotIndex::Slot_3, QuickSlotItem_3);
}

void UAZEquipmentComponent::OnRep_QuickSlot_4()
{
	HandleItemRepNotify(EEquipmentSlot::QuickSlot, ESlotIndex::Slot_4, QuickSlotItem_4);
}