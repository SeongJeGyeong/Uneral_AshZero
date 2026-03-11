// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AZInventoryComponent.h"
#include "Util/AZDefine.h"
#include "AZBagInventoryComponent.generated.h"


class AAZPlayerController;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZBagInventoryComponent : public UAZInventoryComponent
{
	GENERATED_BODY()
public:
	UAZBagInventoryComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Items() override;

	virtual void OnRep_InventoryStruct() override;

public:
	void InitBagInventoryComponent(FBagDefinition BagDefinition);
	void ToggleBagInventory(AAZPlayerController* PlayerController, int32 ID);

	UFUNCTION(NetMulticast, Reliable)
	void InitBagInventoryComponent_Multicast(FBagDefinition BagDefinition);
private:
};
