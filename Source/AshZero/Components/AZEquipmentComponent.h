// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Util/AZDefine.h"
#include "AZEquipmentComponent.generated.h"

class UAZItemBase;
class UAZDragDropOperation;
class UAZBagItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEquipmentSlotUpdated, EEquipmentSlot, SlotType, ESlotIndex, SlotIndex, UAZItemBase*, Item);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZEquipmentComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags);
	UFUNCTION()
	void OnRep_Helmet();
	UFUNCTION()
	void OnRep_Armor();
	UFUNCTION()
	void OnRep_Gloves();
	UFUNCTION()
	void OnRep_Boots();
	UFUNCTION()
	void OnRep_Backpack();
	UFUNCTION()
	void OnRep_FirstWeapon();
	UFUNCTION()
	void OnRep_SecondWeapon();
	UFUNCTION()
	void OnRep_QuickSlot_1();
	UFUNCTION()
	void OnRep_QuickSlot_2();
	UFUNCTION()
	void OnRep_QuickSlot_3();
	UFUNCTION()
	void OnRep_QuickSlot_4();

	UPROPERTY()
	FOnEquipmentSlotUpdated OnEquipmentUpdated;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	ESlotIndex UsedQuickSlotIndex = ESlotIndex::Slot_0;
public:
	void SetUsedQuickSlotIndex(ESlotIndex NewUsedQuickSlotIndex);
	void SubUsedQuickSlotItem();
	TObjectPtr<UAZItemBase>& GetItemRefBySlot(EEquipmentSlot SlotType, ESlotIndex SlotIndex = ESlotIndex::Slot_0);
	
	void EquipItem(EEquipmentSlot SlotType, UAZDragDropOperation* AZInOperation);

	UFUNCTION(Server, Reliable)
	void EquipItem_Server(EEquipmentSlot SlotType, UAZItemBase* NewItem, UAZInventoryComponent* InventoryComponent, int32 ItemLastIndex, EItemSourceType ItemSourceType, ESlotIndex SlotIndex, UAZEquipmentSlot* SourceEquipmentSlot);
	void HandleItemRepNotify(EEquipmentSlot SlotType, ESlotIndex SlotIndex, UAZItemBase* Item);
	void UnEquipItem(EEquipmentSlot SlotType, ESlotIndex SlotIndex);

	UFUNCTION(Server, Reliable)
	void UnEquipItem_Server(EEquipmentSlot SlotType, ESlotIndex SlotIndex);

	void SwapWeaponItem();

	void SwapQuickSlotItem(EItemSourceType ItemSourceType, ESlotIndex SlotIndex);

	void TryStackQuickSlotItem(EItemSourceType ItemSourceType, ESlotIndex SlotIndex);

	TObjectPtr<UAZItemBase>& GetQuickSlotItemRef(ESlotIndex SlotIndex);
	TObjectPtr<UAZItemBase>& GetQuickSlotItemRef(EItemSourceType ItemSourceType);

	UFUNCTION(NetMulticast, Reliable)
	void SetStackQuickSlotItemsCount_NetMulticast(EItemSourceType ItemSourceType, ESlotIndex SlotIndex, int32 SourceCount, int32 DestCount);

	void SubQuickSlotItem(ESlotIndex SlotIndex);
	UFUNCTION(Server, Reliable)
	void SubQuickSlotItem_Server(ESlotIndex SlotIndex);
	UFUNCTION(NetMulticast, Reliable)
	void SetQuickSlotStackCount_Multicast(ESlotIndex SlotIndex, int32 Count);


	// 복제하는 스탯 업데이트
	UFUNCTION(Server, Reliable)
	void UpdateReplicatedStats_Server();
	// 복제하지 않는 스탯 업데이트
	void UpdateLocalStats();

	void EquipBagItem(UAZBagItem* NewBagItem, UAZInventoryComponent* SourceInventory);

	UFUNCTION(Server, Reliable)
	void EquipBagItem_Server(UAZBagItem* NewBagItem, UAZInventoryComponent* SourceInventory);

	UFUNCTION(Client, Reliable)
	void RefreshSourceInventory_Client(UAZInventoryComponent* SourceInventory);

	void RefreshSlots();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(ReplicatedUsing = OnRep_Helmet, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> Helmet;

	UPROPERTY(ReplicatedUsing = OnRep_Armor, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> Armor;

	UPROPERTY(ReplicatedUsing = OnRep_Gloves, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> Gloves;

	UPROPERTY(ReplicatedUsing = OnRep_Boots, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> Boots;

	UPROPERTY(ReplicatedUsing = OnRep_FirstWeapon, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> FirstWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondWeapon, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> SecondWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Backpack, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> Backpack;

	UPROPERTY(ReplicatedUsing = OnRep_QuickSlot_1, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> QuickSlotItem_1;

	UPROPERTY(ReplicatedUsing = OnRep_QuickSlot_2, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> QuickSlotItem_2;

	UPROPERTY(ReplicatedUsing = OnRep_QuickSlot_3, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> QuickSlotItem_3;

	UPROPERTY(ReplicatedUsing = OnRep_QuickSlot_4, BlueprintReadOnly, Category = "AZ|Equipment")
	TObjectPtr<UAZItemBase> QuickSlotItem_4;
};
