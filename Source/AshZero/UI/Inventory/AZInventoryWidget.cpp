// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/AZInventoryWidget.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "Components/GridPanel.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AZInventoryComponent.h"
#include "Blueprint/DragDropOperation.h"
#include "DataAsset/AZBagShapeDataAsset.h"
#include "Components/Overlay.h"
#include "Components/Widget.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/BackgroundBlur.h"
#include "UI/Inventory/AZEquipmentSlot.h"
#include "Components/ProgressBar.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/AZHealthComponent.h"
#include "System/AZSessionSubsystem.h"
#include "GameState/AZLobbyGameState.h"
#include "GameState/AZInGameGameState.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "System/Player/AZPlayerController.h"
#include "Components/AZEquipmentComponent.h"
#include "System/AZDataManagerSubsystem.h"
#include "Components/Image.h"
#include "Item/AZItemBase.h"
#include "Components/AZPlayerInventoryComponent.h"
#include "Components/TextBlock.h"

void UAZInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	LootInventoryGrid->SetVisibility(ESlateVisibility::Collapsed);
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (OwningPawn == nullptr)
		return;
	AAZPlayerCharacter* PlayerCharacter = Cast<AAZPlayerCharacter>(OwningPawn);
	if (PlayerCharacter == nullptr)
		return;
	UAZHealthComponent* HealthComponent = PlayerCharacter->FindComponentByClass<UAZHealthComponent>();
	if (HealthComponent == nullptr)
		return;
	if (PlayerCharacter->IsLocallyControlled())
	{
		HealthComponent->OnSurvivalStatsChanged.AddDynamic(this, &UAZInventoryWidget::UpdateSurvivalStats);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UAZInventoryWidget::OnCloseButtonClicked);
	}
	if (SelectedInventoryVerticalBox)
	{
		SelectedInventoryVerticalBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	UGameInstance* GI = GetGameInstance();
	if (GI == nullptr)
		return;

	UAZSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UAZSessionSubsystem>();
	if (SessionSubsystem == nullptr)
		return;

	SetUIMode(SessionSubsystem->CurrentGameState);

	//SessionSubsystem->OnGameStateChanged.RemoveDynamic(this, &UAZInventoryWidget::SetUIMode);
	//SessionSubsystem->OnGameStateChanged.AddDynamic(this, &UAZInventoryWidget::SetUIMode);
}

FReply UAZInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	/*UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UAZSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UAZSessionSubsystem>();
		if (SessionSubsystem)
		{
			if (SessionSubsystem->CurrentGameState == EGameState::Lobby)
				return FReply::Unhandled();
		}
	}*/
	

	return FReply::Handled();
}

bool UAZInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return false;
}

void UAZInventoryWidget::SetUIMode(EGameState NewMode)
{
	if (InventoryLayouts.Contains(NewMode))
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(InventoryVerticalBox->Slot);
		ApplyLayoutToWidget(CanvasSlot, InventoryLayouts[NewMode]);
	}

	if (FirstGunSlotLayouts.Contains(NewMode))
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(FirstWeaponVerticalBox->Slot);
		ApplyLayoutToWidget(CanvasSlot, FirstGunSlotLayouts[NewMode]);
	}

	if (SecondGunSlotLayouts.Contains(NewMode))
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SecondWeaponVerticalBox->Slot);
		ApplyLayoutToWidget(CanvasSlot, SecondGunSlotLayouts[NewMode]);
	}

	//Å×½ºÆ® ¿ë
	if (UGameplayStatics::GetCurrentLevelName(GetWorld(), true) == TEXT("KSH_TestMap"))
	{
		StashVerticalBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (NewMode == EGameState::Lobby)
	{
		StashVerticalBox->SetVisibility(ESlateVisibility::Visible);
		StashInventoryGrid->SetVisibility(ESlateVisibility::Visible);
		if (UGameplayStatics::GetCurrentLevelName(GetWorld(), true) == TEXT("LobbyLevel_Party"))
		{
			CloseButton->SetVisibility(ESlateVisibility::Visible);
			//BackgroundBorder->SetBrushColor(FLinearColor(1, 1, 1, 0));
			//BackgroundBlur->SetBlurStrength(0.0f);
			Canvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			BackgroundBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
			BackgroundBlur->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	else
	{
		StashVerticalBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAZInventoryWidget::ApplyLayoutToWidget(UCanvasPanelSlot* CanvasSlot, const FWidgetLayoutData& LayoutData)
{
	if (CanvasSlot == nullptr)
		return;

	CanvasSlot->SetAnchors(LayoutData.Anchors);
	CanvasSlot->SetOffsets(LayoutData.Offsets);
	CanvasSlot->SetAlignment(LayoutData.Alignment);
	CanvasSlot->SetPosition(LayoutData.Position);
}

void UAZInventoryWidget::InitInventoryWidget(UAZEquipmentComponent* EquipmentComponent)
{
	LootInventoryGrid->SetVisibility(ESlateVisibility::Collapsed);

	if (EquipmentComponent == nullptr)
		return;
	EquipmentComponent->OnEquipmentUpdated.AddDynamic(this, &UAZInventoryWidget::OnEquipmentChanged);

	OnEquipmentChanged(EEquipmentSlot::Helmet, ESlotIndex::Slot_0, EquipmentComponent->Helmet);
	OnEquipmentChanged(EEquipmentSlot::Armor, ESlotIndex::Slot_0, EquipmentComponent->Armor);
	OnEquipmentChanged(EEquipmentSlot::Gloves, ESlotIndex::Slot_0, EquipmentComponent->Gloves);
	OnEquipmentChanged(EEquipmentSlot::Boots, ESlotIndex::Slot_0, EquipmentComponent->Boots);

	// ¹«±â ½½·Ô °»½Å
	OnEquipmentChanged(EEquipmentSlot::Weapon, ESlotIndex::Slot_0, EquipmentComponent->FirstWeapon);
	OnEquipmentChanged(EEquipmentSlot::Weapon, ESlotIndex::Slot_1, EquipmentComponent->SecondWeapon);

	// Äü½½·Ô °»½Å
	OnEquipmentChanged(EEquipmentSlot::QuickSlot, ESlotIndex::Slot_1, EquipmentComponent->QuickSlotItem_1);
	OnEquipmentChanged(EEquipmentSlot::QuickSlot, ESlotIndex::Slot_2, EquipmentComponent->QuickSlotItem_2);
	OnEquipmentChanged(EEquipmentSlot::QuickSlot, ESlotIndex::Slot_3, EquipmentComponent->QuickSlotItem_3);
	OnEquipmentChanged(EEquipmentSlot::QuickSlot, ESlotIndex::Slot_4, EquipmentComponent->QuickSlotItem_4);
}

void UAZInventoryWidget::UpdateSurvivalStats(float CurrentFullness, float MaxFullness, float CurrentHydration, float MaxHydration)
{
	if (ThirstyProgressBar != nullptr)
	{
		ThirstyProgressBar->SetPercent(CurrentHydration / MaxHydration);
	}
	if (HungerProgressBar != nullptr)
	{
		HungerProgressBar->SetPercent(CurrentFullness / MaxFullness);
	}
}

void UAZInventoryWidget::OnEquipmentChanged(EEquipmentSlot SlotType, ESlotIndex SlotIndex, UAZItemBase* Item)
{
	UAZEquipmentSlot* TargetWidgetSlot = nullptr;
	switch (SlotType)
	{
	case EEquipmentSlot::Helmet:	TargetWidgetSlot = HelmetSlot; break;
	case EEquipmentSlot::Armor:		TargetWidgetSlot = ArmorSlot; break;
	case EEquipmentSlot::Gloves:	TargetWidgetSlot = GlovesSlot; break;
	case EEquipmentSlot::Boots:		TargetWidgetSlot = BootsSlot; break;
	case EEquipmentSlot::Weapon:	TargetWidgetSlot = (SlotIndex == ESlotIndex::Slot_0 ? FirstWeaponSlot : SecondWeaponSlot); break;
	case EEquipmentSlot::QuickSlot: 
		switch (SlotIndex)
		{
		case ESlotIndex::Slot_1: TargetWidgetSlot = QuickSlot_1; break;
		case ESlotIndex::Slot_2:	TargetWidgetSlot = QuickSlot_2; break;
		case ESlotIndex::Slot_3: TargetWidgetSlot = QuickSlot_3; break;
		case ESlotIndex::Slot_4: TargetWidgetSlot = QuickSlot_4; break;
		default: break;
		}
		break;
	default: break;
	}

	if (TargetWidgetSlot == nullptr)
		return;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;

	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	if (Item == nullptr)
	{
		TargetWidgetSlot->ItemImage->SetBrushFromMaterial(nullptr);
		TargetWidgetSlot->ItemImage->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
		TargetWidgetSlot->StackCount->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	UMaterialInstance* MaterialInstance = DataManger->GetMaterialByID(Item->GetItemID());
	if (MaterialInstance == nullptr)
		return;
	TargetWidgetSlot->ItemImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	TargetWidgetSlot->ItemImage->SetBrushFromMaterial(MaterialInstance);
	if (Item->GetStackCount() > 1)
	{
		TargetWidgetSlot->StackCount->SetVisibility(ESlateVisibility::HitTestInvisible);
		TargetWidgetSlot->StackCount->SetText(FText::AsNumber(Item->GetStackCount()));
	}
	
}

void UAZInventoryWidget::OnCloseButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		//FInputModeGameAndUI InputMode;
		//PC->SetInputMode(InputMode);
		//PC->bShowMouseCursor = true;
		AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC);
		if (AZPC)
		{
			AZPC->SwitchPartyUI();
		}
	}
	SetVisibility(ESlateVisibility::Collapsed);
}
