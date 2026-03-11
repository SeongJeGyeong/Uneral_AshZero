// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Util/AZDefine.h"
#include "CoreMinimal.h"
#include "Components/AZInventoryComponent.h"
#include "AZLootBoxComponent.generated.h"

class UAZItemBase;
class UAZChestGridWidget;
struct FAZDropDataTable;
struct FAZItemDataTableRow;



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZLootBoxComponent : public UAZInventoryComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZLootBoxComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Items() override;

	virtual void OnRep_InventoryRevision() override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GenerateRandomItems();
	void TestGenerate();

	UPROPERTY(EditAnywhere, Category = "AZ")
	ELootSourceType LootSourceType;

	UPROPERTY(Replicated)
	bool bIsItemsGenerated = false;

	void AddReward(const FAZQuestReward& Reward);

private:
	void GenerateRandomItemsByDropTable(TArray<FAZDropDataTable*> SelectedDropTable);

};
