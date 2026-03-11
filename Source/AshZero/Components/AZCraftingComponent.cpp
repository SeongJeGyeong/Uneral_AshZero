// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AZCraftingComponent.h"
#include "System/Player/AZPlayerController.h"
#include "System/AZDataManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "DataTable/AZCraftingRecipeDataTableRow.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Crafting/AZCraftingWidget.h"
#include "UI/Crafting/AZCraftingListItemWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Components/AZStashComponent.h"
#include "Item/AZItemBase.h"

// Sets default values for this component's properties
UAZCraftingComponent::UAZCraftingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UAZCraftingComponent::Interact_Implementation(AActor* InstigatorActor)
{
	if (InstigatorActor == nullptr)
		return;
	if (bIsInitialized == true)
	{
		if (LinkedStashComponent != nullptr)
		{
			LinkedStashComponent->BroadcastCurrentStashState();
		}
		return;
	}

	AAZPlayerController* AZPC = Cast<AAZPlayerController>(InstigatorActor->GetInstigatorController());
	if (AZPC == nullptr)
		return;

	InitCraftingComponent(AZPC);
}


// Called when the game starts
void UAZCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UAZCraftingComponent::InitCraftingComponent(AAZPlayerController* AZPlayerController)
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;
	UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (!DataManager || !AZPlayerController || !AZPlayerController->CraftingWidget) return;

	UAZCraftingWidget* CraftingWidget = AZPlayerController->CraftingWidget;
	TArray<FAZCraftingRecipeDataTableRow*> AllCraftingRecipe = DataManager->GetAllCraftingRecipe();

	if (AllCraftingRecipe.Num() == 0) return;

	if (!CraftingWidget->CraftingListScroll || !CraftingListItemWidgetClass || !AZPlayerController->StashComp) return;

	LinkedStashComponent = AZPlayerController->StashComp;
	if (LinkedStashComponent == nullptr)
		return;
	const FItemAmountMap& AmountMap = LinkedStashComponent->GetCurrentAmounts();
	for (FAZCraftingRecipeDataTableRow* CraftingRecipeRow : AllCraftingRecipe)
	{
		CraftingRecipes.Add(CraftingRecipeRow->JobID, CraftingRecipeRow);
		UAZCraftingListItemWidget* CraftingListItemWidget = CreateWidget<UAZCraftingListItemWidget>(GetWorld(), CraftingListItemWidgetClass);
		if (CraftingListItemWidget == nullptr)
			continue;
	
		CraftingListItemWidget->Setup(this, CraftingRecipeRow, AmountMap);

		CraftingWidget->CraftingListScroll->AddChild(CraftingListItemWidget);
		CraftingWidget->AddToViewport();

		LinkedStashComponent->OnStashItemStateChanged.AddDynamic(CraftingListItemWidget, &UAZCraftingListItemWidget::UpdateAvailability);
	}

	bIsInitialized = true;
}

bool UAZCraftingComponent::TryStartCrafting(int32 JobID)
{
	FAZCraftingRecipeDataTableRow* CraftingRecipe = CraftingRecipes[JobID];
	if (ActiveCraftingTimers.Contains(JobID))
		return false;
	if (CraftingRecipe == nullptr) return false;
	if (LinkedStashComponent->HasIngredients(CraftingRecipe))
	{
		FTimerHandle NewHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUObject(this, &UAZCraftingComponent::FinishCrafting, JobID);
		GetWorld()->GetTimerManager().SetTimer(
			NewHandle,
			TimerDel,
			CraftingRecipe->Craft_Time_sec,
			false
		);

		ActiveCraftingTimers.Add(JobID, NewHandle);
		LinkedStashComponent->RemoveItemQuantity(CraftingRecipes[JobID]);
		LinkedStashComponent->OnRep_Items();
		LinkedStashComponent->BroadcastCurrentStashState();
		return true;
	}
	return false;
}

bool UAZCraftingComponent::CollectCraftedItem(int32 JobID)
{
	if (ReadyToCollectJobs.Contains(JobID))
	{
		if (!CraftingRecipes.Contains(JobID)) return false;

		UAZItemBase* NewItem = NewObject<UAZItemBase>(this);
		NewItem->SetItemID(CraftingRecipes[JobID]->ResultItemID);
		LinkedStashComponent->TryAddItem(NewItem);
		LinkedStashComponent->OnRep_Items();

		ReadyToCollectJobs.Remove(JobID);

		OnCraftingFinished.Broadcast(JobID);
		return true;
	}
	return false;
}

bool UAZCraftingComponent::IsCraftingReadyToCollect(int32 JobID) const
{
	return ReadyToCollectJobs.Contains(JobID);
}

void UAZCraftingComponent::FinishCrafting(int32 JobID)
{
	if (ActiveCraftingTimers.Contains(JobID))
	{
		ActiveCraftingTimers.Remove(JobID);

		if (!CraftingRecipes.Contains(JobID))
			return;

		ReadyToCollectJobs.Add(JobID);

		OnCraftingReadyToCollect.Broadcast(JobID);

		//UAZItemBase* NewItem = NewObject<UAZItemBase>(this);
		//NewItem->SetItemID(CraftingRecipes[JobID]->ResultItemID);
		//LinkedStashComponent->TryAddItem(NewItem);
		//LinkedStashComponent->OnRep_Items();
		//OnCraftingFinished.Broadcast(JobID);
	}
}

bool UAZCraftingComponent::IsCraftingItem(int32 JobID) const
{
	return ActiveCraftingTimers.Contains(JobID);
}

float UAZCraftingComponent::GetCraftingProgress(int32 JobID) const
{
	if (!ActiveCraftingTimers.Contains(JobID)) return 0.0f;

	FTimerHandle Handle = ActiveCraftingTimers[JobID];
	float TotalTime = CraftingRecipes[JobID]->Craft_Time_sec;
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(Handle);

	return (TotalTime > 0.f) ? (ElapsedTime / TotalTime) : 1.0f;
}

float UAZCraftingComponent::GetRemainingTime(int32 JobID) const
{
	if (!ActiveCraftingTimers.Contains(JobID)) return 0.0f;
	return GetWorld()->GetTimerManager().GetTimerRemaining(ActiveCraftingTimers[JobID]);
}



