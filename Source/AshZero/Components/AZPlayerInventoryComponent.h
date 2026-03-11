// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AZInventoryComponent.h"
#include "AZPlayerInventoryComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ASHZERO_API UAZPlayerInventoryComponent : public UAZInventoryComponent
{
	GENERATED_BODY()
public:
	UAZPlayerInventoryComponent();
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_InventoryStruct() override;
	virtual void OnRep_Items() override;
protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

	//void OnEquipmentChanged(EEquipmentSlot SlotType, ESlotIndex SlotIndex, UAZItemBase* Item);
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitPlayerInventoryComponent();
	void SetupInputBinding(class UEnhancedInputComponent* PlayerInputComponent);

	void InputToggleInventory(const struct FInputActionValue& Value);
	void ToggleInventory();

	void OpenLobbyInventory();
	UFUNCTION(Server, Reliable)
	void InitInventory_Server(FBagDefinition NewBagShape, int32 Size);

	void UpdateBagImage();
	int32 GetItemCount(int32 ItemID);

	//UFUNCTION(Server, Reliable)
	//void ChangeInventoryData_Server();
	bool bInitialized = false;
};
