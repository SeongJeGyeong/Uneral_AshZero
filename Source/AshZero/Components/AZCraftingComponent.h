// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/AZInteractable.h"
#include "AZCraftingComponent.generated.h"

class UAZCraftingListItemWidget;
class AAZPlayerController;
class UAZStashComponent;
struct FAZCraftingRecipeDataTableRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingFinished, int32, CompletedJobID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingReadyToCollect, int32, JobID);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZCraftingComponent : public UActorComponent, public IAZInteractable
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZCraftingComponent();
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	TMap<int32, FTimerHandle> ActiveCraftingTimers;
public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(EditAnywhere, Category = "AZ|UI")
	TSubclassOf<UAZCraftingListItemWidget> CraftingListItemWidgetClass;

	void InitCraftingComponent(AAZPlayerController* AZPlayerController);

	bool TryStartCrafting(int32 JobID);

	bool IsCraftingItem(int32 JobID) const;

	float GetCraftingProgress(int32 JobID) const;

	float GetRemainingTime(int32 JobID) const;

	UPROPERTY()
	FOnCraftingFinished OnCraftingFinished;
	UPROPERTY()
	FOnCraftingReadyToCollect OnCraftingReadyToCollect;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AZ|Link")
	TObjectPtr<AActor> LinkedStorageActor;

	UFUNCTION(BlueprintCallable, Category = "AZ|Crafting")
	bool CollectCraftedItem(int32 JobID);

	UFUNCTION(BlueprintPure, Category = "AZ|Crafting")
	bool IsCraftingReadyToCollect(int32 JobID) const;
private:
	void FinishCrafting(int32 JobID);

	bool bIsInitialized = false;
	TMap<int32, FAZCraftingRecipeDataTableRow*> CraftingRecipes;
	TSet<int32> ReadyToCollectJobs;
	UPROPERTY(VisibleAnywhere, Category = "AZ|Component")
	TObjectPtr<UAZStashComponent> LinkedStashComponent;
};
