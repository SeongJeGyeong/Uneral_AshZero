// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Util/AZDefine.h"
#include "System/AZDataManagerSubsystem.h"
#include "AZItemBase.generated.h"

struct FAZBaseItemDataTable;
/**
 * 
 */
UCLASS(Blueprintable)
class ASHZERO_API UAZItemBase : public UObject
{
	GENERATED_BODY()
public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;
public:
	UFUNCTION()
	virtual void OnRep_ItemID();
	FIntPoint GetDimensions() const;

	UMaterialInterface* GetIcon();

	bool GetIsRotated();

	void SetIsRotated(bool bIsRotate);

	bool GetIsDragged();

	void SetIsDragged(bool bIsDrag);

	void RotateItem();

	void SetItemID(int32 NewItemID);
	int32 GetItemID();

	void SetStackCount(int32 NewStackCount);
	int32 GetStackCount();


	virtual void ApplyItemData(const FAZBaseItemDataTable* ItemData);
	void SetItemData(int32 NewItemID);

	EItemType GetItemType();
	EEquipmentSlot GetEquipmentSlotType();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "AZ|Items")
	bool bIsStarterItem = false;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ItemID)
	int32 ItemID;

	UPROPERTY(EditAnywhere, Category = "AZ|Items")
	FIntPoint Dimensions;

	UPROPERTY(EditAnywhere, Category = "AZ|Items")
	UMaterialInterface* Icon;	

	UPROPERTY(VisibleDefaultsOnly, Category = "AZ|Items")
	EItemType ItemType;

	UPROPERTY(Replicated, VisibleDefaultsOnly, Category = "AZ|Items")
	int32 StackCount = 1;

	UPROPERTY(VisibleDefaultsOnly, Category = "AZ|Items")
	EEquipmentSlot EquipSlot;

	UPROPERTY(Replicated, VisibleDefaultsOnly ,Category = "AZ|Items")
	bool bIsRotated = false;

	UPROPERTY(Replicated, EditAnywhere, Category = "AZ|Items")
	bool bIsDragged = false;
};
