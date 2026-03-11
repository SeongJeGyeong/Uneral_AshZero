// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AZBagInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "System/AZDataManagerSubsystem.h"
#include "Character/AZPlayerCharacter.h"
#include "System/Player/AZPlayerController.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "UI/Inventory/AZBagSlotWidget.h"

UAZBagInventoryComponent::UAZBagInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UAZBagInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAZBagInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAZBagInventoryComponent, Items);
}

void UAZBagInventoryComponent::OnRep_Items()
{
	Super::OnRep_Items();
}

void UAZBagInventoryComponent::OnRep_InventoryStruct()
{
	Super::OnRep_InventoryStruct();

	//if (InventoryGridWidget == nullptr)
	//	return;
	//if (InventoryGridWidget->ItemsCanvasPanel != nullptr)
	//{
	//	InventoryGridWidget->SetVisibility(ESlateVisibility::Collapsed);
	//}

}	

void UAZBagInventoryComponent::InitBagInventoryComponent(FBagDefinition BagDefinition)
{
	InventoryStruct = BagDefinition;
	Items.SetNum(InventoryStruct.Columns * InventoryStruct.Rows);
	//InitBagInventoryComponent_Multicast(BagDefinition);
}

void UAZBagInventoryComponent::InitBagInventoryComponent_Multicast_Implementation(FBagDefinition BagDefinition)
{
	InventoryStruct = BagDefinition;
	Items.SetNum(InventoryStruct.Columns * InventoryStruct.Rows);
}

void UAZBagInventoryComponent::ToggleBagInventory(AAZPlayerController* PlayerController, int32 ID)
{
	if (PlayerController == nullptr)
		return;

	UAZInventoryWidget* InvenWidget = PlayerController->GetInventoryWidget();
	if (InvenWidget == nullptr)
		return;
	if (InvenWidget->BagInventoryGrid == nullptr)
		return;
	InventoryGridWidget = InvenWidget->BagInventoryGrid;

	
	if (InvenWidget->SelectedInventoryVerticalBox->GetVisibility() == ESlateVisibility::Visible)
	{
		InventoryGridWidget->ItemsCanvasPanel->ClearChildren();
		InventoryGridWidget->GridCanvasPanel->ClearChildren();
		if (InventoryGridWidget->IsCurrentInventoryComponent(this))
		{
			InvenWidget->SelectedInventoryVerticalBox->SetVisibility(ESlateVisibility::Collapsed);
			InventoryGridWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			InventoryGridWidget->Init(this);
			InventoryGridWidget->CreateInvnetorySlot();
			InventoryGridWidget->CreateAllItemWidget(this);
		}

	}
	else if (InvenWidget->SelectedInventoryVerticalBox->GetVisibility() == ESlateVisibility::Collapsed)
	{
		InventoryGridWidget->Init(this);
		InventoryGridWidget->CreateInvnetorySlot();
		InventoryGridWidget->CreateAllItemWidget(this);
		InventoryGridWidget->SetVisibility(ESlateVisibility::Visible);
		InvenWidget->SelectedInventoryVerticalBox->SetVisibility(ESlateVisibility::Visible);

		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			if (UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>())
			{
				if (UMaterialInstance* MI = DataManager->GetMaterialByID(ID))
				{
					if (InvenWidget && InvenWidget->SelectBagSlot && InvenWidget->SelectBagSlot->ItemImage)
					{
						InvenWidget->SelectBagSlot->ItemImage->SetBrushFromMaterial(MI);
					}
				}
			}
		}
	}
}


