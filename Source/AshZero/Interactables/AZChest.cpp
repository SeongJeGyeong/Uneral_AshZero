// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/AZChest.h"
#include "Components/AZLootBoxComponent.h"
#include "Components/AZInventoryComponent.h"
#include "Components/AZPlayerInventoryComponent.h"
#include "Components/AZBagInventoryComponent.h"
#include "Character/AZPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/AZHUD.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "System/Player/AZPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Item/AZItemBase.h"
#include "Item/AZBagItem.h"
#include "Util/AZDefine.h"
#include "Components/Overlay.h"
#include "Components/Widget.h"
#include "Engine/ActorChannel.h"
#include "Components/StaticMeshComponent.h" 
#include "Materials/MaterialInstanceDynamic.h"
#include "System/Player/Components/AZInteractionUIComponent.h"

// Sets default values
AAZChest::AAZChest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	LootBoxComponent = CreateDefaultSubobject<UAZLootBoxComponent>(TEXT("LootBoxComponent"));
	HighlightColor = EHighlightColor::Yellow;
	InteractionText = FText::FromString(TEXT("상자 열기"));
}

bool AAZChest::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (LootBoxComponent != nullptr)
	{
		bWroteSomething |= LootBoxComponent->ReplicateSubobjectsFromItems(Channel, Bunch, RepFlags);
	}

	for (UAZItemBase* Item : LootBoxComponent->Items)
	{
		if (Item == nullptr)
			continue;
		if (Item->GetItemType() != EItemType::Storage)
			continue;
		UAZBagItem* BagItem = Cast<UAZBagItem>(Item);
		if (BagItem == nullptr)
			continue;
		bWroteSomething |= Channel->ReplicateSubobject(BagItem->InventoryComponent, *Bunch, *RepFlags);
		bWroteSomething |= BagItem->InventoryComponent->ReplicateSubobjectsFromItems(Channel, Bunch, RepFlags);
	}
	
	return bWroteSomething;
}

//void AAZChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	DOREPLIFETIME(AAZChest, bIsFirstInteract);
//}

// Called when the game starts or when spawned
void AAZChest::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<UStaticMeshComponent*> MeshComponents;
	GetComponents<UStaticMeshComponent>(MeshComponents);

	for (UStaticMeshComponent* Mesh : MeshComponents)
	{
		const int32 Matcount = Mesh->GetNumMaterials();
		for (int32 i = 0; i < Matcount; i++)
		{
			UMaterialInstanceDynamic* MID = Mesh->CreateAndSetMaterialInstanceDynamic(i);
			if (MID)
			{
				CachedMIDs.Add(MID);
			}
		}
	}

	if (DissolveCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleDissolveProgress"));
		DissolveTimeline.AddInterpFloat(DissolveCurve, ProgressFunction);

		FOnTimelineEvent FinishedFunction;
		FinishedFunction.BindUFunction(this, FName("HandleDissolveFinished"));
		DissolveTimeline.SetTimelineFinishedFunc(FinishedFunction);

		DissolveTimeline.SetLooping(false);
		DissolveTimeline.SetPlayRate(1.0f / DissolveDuration);

		HandleDissolveProgress(1.0f);
		DissolveTimeline.ReverseFromEnd();
	}
	
}

void AAZChest::SetIsItemGenerated_Server_Implementation(bool bIsItemGenerate)
{
	bIsItemsGenerated = bIsItemGenerate;
	SetIsItemGenerated_Multicast(bIsItemsGenerated);
}

void AAZChest::SetIsItemGenerated_Multicast_Implementation(bool bIsItemGenerate)
{
	bIsItemsGenerated = bIsItemGenerate;
}

// Called every frame
void AAZChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DissolveTimeline.IsPlaying())
	{
		DissolveTimeline.TickTimeline(DeltaTime);
	}
}

void AAZChest::Interact_Implementation(AActor* InstigatorActor)
{

	AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(InstigatorActor);
	if (Player == nullptr) return;

	if (!Player->IsLocallyControlled()) return;

	AAZPlayerController* PC = Cast<AAZPlayerController>(Player->GetController());
	if (PC == nullptr) return;

	UAZInventoryWidget* InvenWidget = PC->GetInventoryWidget();
	if (InvenWidget == nullptr) return;

	LootBoxComponent->InventoryGridWidget = InvenWidget->LootInventoryGrid;
	if (LootBoxComponent->InventoryGridWidget == nullptr) return;

	//if (HasAuthority())
	//{
	//	if (bIsItemsGenerated == false)
	//	{
	//		LootBoxComponent->GenerateRandomItems();
	//		InitBagItems();
	//		bIsItemsGenerated = true;
	//		SetIsItemGenerated_Multicast(bIsItemsGenerated);
	//	}
	//}
	//else
	//{
	//	if (bIsItemsGenerated == false)
	//	{
	//		PC->RequestOpenLootBox_Server(LootBoxComponent, this);
	//		SetIsItemGenerated_Server(true);
	//	}
	//}

	PC->RequestOpenLootBox_Server(LootBoxComponent, this);

	FInputModeGameAndUI InputMode;

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PC->InventoryWidget->SetVisibility(ESlateVisibility::Visible);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
	PC->InventoryWidget->LootInventoryGrid->Init(LootBoxComponent);
	PC->InventoryWidget->LootInventoryGrid->SetVisibility(ESlateVisibility::Visible);
	LootBoxComponent->RefreshAllItems();
	PC->InventoryWidget->LootInventoryGrid->CreateAllItemWidget(LootBoxComponent);
	
	PC->InventoryWidget->EquipmnetOverlay->SetVisibility(ESlateVisibility::Hidden);

	HighlightColor = EHighlightColor::White;
	PC->InteractionUIComp->TargetHighlighted(this, true);
}

void AAZChest::InitBagItems()
{
	TArray<UAZItemBase*> Items;
	LootBoxComponent->GetAllItems().GetKeys(Items);
	for (UAZItemBase* Item : Items)
	{
		if (Item == nullptr)
			continue;
		if (Item->GetItemType() != EItemType::Storage)
			continue;
		UAZBagItem* BagItem = Cast<UAZBagItem>(Item);
		if (BagItem == nullptr)
			continue;
		UAZBagInventoryComponent* NewBagInventoryComponent = NewObject<UAZBagInventoryComponent>(this);
		if (NewBagInventoryComponent == nullptr)
			continue;
		NewBagInventoryComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		NewBagInventoryComponent->SetIsReplicated(true);
		NewBagInventoryComponent->RegisterComponent();

		AddInstanceComponent(NewBagInventoryComponent);

		NewBagInventoryComponent->ItemWidgetClass = LootBoxComponent->ItemWidgetClass;
		BagItem->InitBagItem(NewBagInventoryComponent);
	}
}

void AAZChest::GenerateRewardBox(const TArray<FAZQuestReward>& Rewards)
{
	bIsItemsGenerated = true;
	for (FAZQuestReward Reward : Rewards)
	{
		LootBoxComponent->AddReward(Reward);
	}
}

void AAZChest::RegisterInteractor(AActor* User)
{
	if (!HasAuthority() || !User) return;

	CurrentInteractors.Add(User);
}

void AAZChest::UnregisterInteractor(AActor* User)
{
	if (!HasAuthority()) return;

	if (CurrentInteractors.Contains(User))
	{
		CurrentInteractors.Remove(User);

		// "마지막 사람이 나갔는데, 이미 상자가 비어있었다면" -> 사라짐
		CheckAndStartDissolve();
	}
}

void AAZChest::HandleDissolveProgress(float Value)
{
	float InvertedAlpha = 1.0f - Value;

	for (UMaterialInstanceDynamic* MID : CachedMIDs)
	{
		if (MID)
		{
			MID->SetScalarParameterValue(FName("DissolveAmount"), Value);
			MID->SetScalarParameterValue(FName("DecalOpacity"), InvertedAlpha);
		}
	}
}

void AAZChest::HandleDissolveFinished()
{
	if (bIsDisappearing)
	{
		Destroy();
	}
}

void AAZChest::CheckAndStartDissolve()
{
	if (!HasAuthority()) 
		return;

	if (bIsDisappearing) return;

	if (IsLootBoxEmpty() && CurrentInteractors.Num() == 0)
	{
		SetActorEnableCollision(false);
		StartDestroyEffect_Multicast();
	}
}

bool AAZChest::IsLootBoxEmpty() const
{
	if (!LootBoxComponent) return false;
	return LootBoxComponent->GetAllItems().Num() == 0;
}

void AAZChest::StartDestroyEffect_Multicast_Implementation()
{
	bIsDisappearing = true;

	if (DissolveTimeline.IsPlaying())
	{
		DissolveTimeline.Stop();
	}

	DissolveTimeline.PlayFromStart();

}
