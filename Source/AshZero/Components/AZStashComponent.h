// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AZInventoryComponent.h"
#include "Util/AZDefine.h"
#include "AZStashComponent.generated.h"


struct FAZCraftingRecipeDataTableRow;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStashItemStateChanged, const FItemAmountMap&, ItemAmounts);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ASHZERO_API UAZStashComponent : public UAZInventoryComponent
{
	GENERATED_BODY()
public:
	UAZStashComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Items() override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitStashComponent();

	bool HasIngredients(FAZCraftingRecipeDataTableRow* CraftingRecipes);

	void RemoveItemQuantity(FAZCraftingRecipeDataTableRow* CraftingRecipes);

	int32 GetTotalItemCount(int32 MaterialID);
	FItemAmountMap GetCurrentAmounts();

	UPROPERTY()
	FOnStashItemStateChanged OnStashItemStateChanged;
	UFUNCTION()
	void BroadcastCurrentStashState();

	int32 RemoveItemByID(int32 ItemID, int32 Quantity);
private:

	void TestGenerateItem();
};
