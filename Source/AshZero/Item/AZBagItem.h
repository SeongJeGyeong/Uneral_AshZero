// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/AZItemBase.h"
#include "AZBagItem.generated.h"

class UAZBagInventoryComponent;
struct FAZBaseItemDataTable;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZBagItem : public UAZItemBase
{
	GENERATED_BODY()
public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;

	virtual void ApplyItemData(const FAZBaseItemDataTable* ItemData) override;

	virtual void OnRep_ItemID() override;

	void InitializeRandomBagData();

	void InitBagItem(UAZBagInventoryComponent* NewInventoryComponent);
	UPROPERTY(Replicated)
	TObjectPtr<UAZBagInventoryComponent> InventoryComponent = nullptr;
	FBagDefinition GetBagData();
	void SetBagData(FBagDefinition NewBagData);
protected:
	UPROPERTY(Replicated)
	FBagDefinition BagData;
};
