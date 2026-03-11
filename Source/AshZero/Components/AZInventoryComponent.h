// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Util/AZDefine.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AZInventoryComponent.generated.h"

class UAZItemBase;
class UAZInventoryGridWidget;
class UAZBagShapeDataAsset;
class UAZItemWidget;
class UAZBagItem;
class UAZBagInventoryComponent;
class UAZEquipmentComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjectsFromItems(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags);
	UFUNCTION()
	virtual void OnRep_Items();
	UFUNCTION()
	virtual void OnRep_InventoryStruct();
	UFUNCTION()
	virtual void OnRep_InventoryRevision();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "AZ|UI")
	float TileSize = 64.0f;

	bool bAddedItem = false;

	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<UAZItemBase*> Items;

	UPROPERTY(ReplicatedUsing = OnRep_InventoryStruct)
	FBagDefinition InventoryStruct;

	UPROPERTY(ReplicatedUsing = OnRep_InventoryRevision)
	int32 InventoryRevision = 0;

	FBagDefinition GetInventoryStruct();

	UPROPERTY()
	TMap<UAZItemBase*, FIntPoint> AllItems;
	
	bool TryAddItem(UAZItemBase* ItemToAdd);
	
	bool IsRoomAvailable(UAZItemBase* ItemToAdd, int32 Index);

	FIntPoint IndexToTile(int32 TopLeftIndex);

	bool IsValidTile(FIntPoint Tile);

	int32 TileToIndex(FIntPoint Tile);

	bool GetResultAtIndex(int32 Index);

	UAZItemBase* GetItemAtIndex(int32 Index);

	int32 GetIndexAtItem(UAZItemBase* Item);
	bool IsVaildIndex(int32 Index);

	void AddItemAt(UAZItemBase* ItemToAdd, int32 TopLeftIndex);

	UFUNCTION(Server, Reliable)
	void AddItemAt_Server(UAZItemBase* ItemToAdd, int32 TopLeftIndex);
	TMap<UAZItemBase*, FIntPoint> GetAllItems();

	

	void SetInventoryGridWidget(UAZInventoryGridWidget* GridWidget);

	void RemoveItem(UAZItemBase* ItemToRemove);
	UFUNCTION(Server, Reliable)
	void RemoveItem_Server(UAZItemBase* ItemToRemove);
	void RefreshAllItems();

	int32 RemoveItemByTag(FGameplayTag GamPlayTag, int32 Amount);

	//UPROPERTY(EditAnywhere, Category = "AZ|UI")
	//TSubclassOf<UAZItemBase> ItemBlueprintClass;

	//UPROPERTY(EditAnywhere, Category = "AZ|UI")
	//TSubclassOf<UUserWidget> InventoryWidgetClass;

	UUserWidget* InventoryWidget;

	UPROPERTY(Replicated)
	TSubclassOf<UAZItemWidget> ItemWidgetClass;

	UPROPERTY()
	TObjectPtr<UAZInventoryGridWidget> InventoryGridWidget;

	UPROPERTY()
	UAZItemWidget* ItemWidget;

	UPROPERTY()
	UAZItemBase* ItemToAdded;

	void SetDraggedItemLastIndex(int32 Index);
	int32 GetDraggedItemLastIndex();
	int32 DraggedItemLastIndex = -1;
	bool bPayloadIsRotated = false;

	//UPROPERTY(EditAnywhere, Category = "AZ|Inventory")
	//TObjectPtr<UAZBagShapeDataAsset> BagShapeDataAsset;

	void InventoryRefresh();

	UFUNCTION(NetMulticast, Reliable)
	void SetItemStack_Multicast(UAZItemBase* Item, int32 Count);

	UFUNCTION(Server, Reliable)
	void ResetInventoryData_Server();

	UPROPERTY(BlueprintAssignable, Category = "AZ|Event")
	FOnInventoryUpdated OnInventoryUpdated;


};
