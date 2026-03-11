// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AZPlayerInventoryComponent.h"
#include "Item/AZItemBase.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "DataAsset/AZBagShapeDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/AZHUD.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "EnhancedInputComponent.h" 
#include "EnhancedInputSubsystems.h"
#include "Character/AZPlayerCharacter.h"
#include "AZDataAsset.h"
#include "AZGameplayTags.h"
#include "System/Player/AZPlayerController.h"
#include "System/AZDataManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/CanvasPanel.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Components/Overlay.h"
#include "Components/Widget.h"
#include "System/AZSessionSubsystem.h"
#include "Components/AZEquipmentComponent.h"
#include "Components/Image.h"
#include "Item/AZBagItem.h"
#include "Components/AZBagInventoryComponent.h"
#include "GameFramework/Actor.h"
#include "Components/VerticalBox.h"
#include "UI/Inventory/AZBagSlotWidget.h"

UAZPlayerInventoryComponent::UAZPlayerInventoryComponent()
{
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UAZPlayerInventoryComponent::OnRep_InventoryStruct()
{
	AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
	if (PC == nullptr)
		return;
	if (!PC->IsLocalController())
		return;
	UAZInventoryWidget* InvenWidget = PC->GetInventoryWidget();
	if (InvenWidget == nullptr)
		return;

	InventoryGridWidget = InvenWidget->PlayerInventoryGrid;
	//InventoryGridWidget = HUD->InventoryWidget->PlayerInventoryGrid;
	if (InventoryGridWidget == nullptr)
		return;
	InventoryGridWidget->Init(this);
	//InventoryGridWidget->CreateInvnetorySlot();
	Super::OnRep_InventoryStruct();
	bInitialized = true;
}

void UAZPlayerInventoryComponent::OnRep_Items()
{
	Super::OnRep_Items();

}

//void UAZPlayerInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	DOREPLIFETIME_CONDITION(UAZInventoryComponent, Items, COND_OwnerOnly);
//}

void UAZPlayerInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
}

void UAZPlayerInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UAZPlayerInventoryComponent::InitPlayerInventoryComponent()
{

	AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
	if (PC == nullptr)
		return;

	if (!PC->IsLocalController())
		return;

	//if (bInitialized == true)
	//{
	//	if (UAZInventoryWidget* InvenWidget = PC->GetInventoryWidget())
	//	{
	//		if (!InvenWidget->IsInViewport())
	//		{
	//			InvenWidget->AddToViewport();
	//		}
	//	}
	//	return;
	//}

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	UAZEquipmentComponent* EquipComp = PC->EquipmentComp;
	UAZItemBase* CurrentBackpack = nullptr;
	if (EquipComp == nullptr)
		return;

	/*EquipComp->OnEquipmentUpdated.RemoveDynamic(this, &UAZPlayerInventoryComponent::OnEquipmentChanged);
	EquipComp->OnEquipmentUpdated.AddDynamic(this, &UAZPlayerInventoryComponent::OnEquipmentChanged);*/
		
	CurrentBackpack = EquipComp->GetItemRefBySlot(EEquipmentSlot::Backpack, ESlotIndex::Slot_0);
	

	
	//로비에서 가방 아이템을 장착하고 들어올 때 로직 따로 구현

	//로비에서 가방 아이템을 장착하지 않았을 때 기본으로 이 가방을 장착
	if (CurrentBackpack == nullptr)
	{
		const FBagDefinition* BagShape = DataManger->GetBagShapeByName("603_1");
		if (BagShape)
		{
			if (InventoryStruct == *BagShape)
			{
				OnRep_InventoryStruct();
				OnRep_Items();
			}
			else
			{
				InitInventory_Server(*BagShape, BagShape->Columns * BagShape->Rows);
			}
		}
	}
	else
	{
		OnRep_InventoryStruct();
		OnRep_Items();
		UpdateBagImage();
	}
	//else
	//{
	//	UAZBagItem* BackpackItem = Cast<UAZBagItem>(CurrentBackpack);
	//	if (InventoryStruct == BackpackItem->GetBagData())
	//	{
	//		InventoryStruct = BackpackItem->GetBagData();
	//		Items.SetNum(InventoryStruct.Columns * InventoryStruct.Rows);
	//		OnRep_InventoryStruct();
	//		OnRep_Items();
	//	}
	//	else
	//	{
	//		FBagDefinition NewBagData = BackpackItem->GetBagData();
	//		InitInventory_Server(BackpackItem->GetBagData(), NewBagData.Columns * NewBagData.Rows);
	//	}
	//}
}

void UAZPlayerInventoryComponent::InitInventory_Server_Implementation(FBagDefinition NewBagShape, int32 Size)
{
	AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
	if (PC == nullptr)
		return;
	UAZEquipmentComponent* EquipComp = PC->EquipmentComp;
	UAZItemBase* CurrentBackpack = nullptr;
	if (EquipComp == nullptr)
		return;
	UAZBagItem* NewBackpack = NewObject<UAZBagItem>(this);

	if (NewBackpack)
	{
		NewBackpack->SetItemData(NewBagShape.ID);
		NewBackpack->SetBagData(NewBagShape);
		EquipComp->Backpack = NewBackpack;
		InventoryStruct = NewBagShape;
		Items.SetNum(Size);

		UAZBagInventoryComponent* NewBagInventoryComponent = NewObject<UAZBagInventoryComponent>(this);
		if (NewBagInventoryComponent == nullptr)
			return;
		NewBagInventoryComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		NewBagInventoryComponent->SetIsReplicated(true);
		NewBagInventoryComponent->RegisterComponent();

		NewBackpack->InitBagItem(NewBagInventoryComponent);
		OnRep_InventoryStruct();
		OnRep_Items();
	}
	
}

void UAZPlayerInventoryComponent::SetupInputBinding(class UEnhancedInputComponent* PlayerInputComponent)
{
	if(PlayerInputComponent == nullptr) return;
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC == nullptr)
		return;

	AAZPlayerCharacter* OwnerCharacter = Cast<AAZPlayerCharacter>(PC->GetPawn());
	if (OwnerCharacter == nullptr) return;

	// 로컬 컨트롤러인지 확인 (서버의 AI 등은 입력 불필요)
	//if (!OwnerCharacter->IsLocallyControlled()) return;

	UAZDataAsset* InputData = OwnerCharacter->InputDataAsset;
	if (InputData == nullptr) return;

	// 1. 매핑 컨텍스트 추가 (컨트롤러가 확실히 존재할 때 실행됨)

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputData->IMC_TPS, 0);
	}
	

	// 2. 액션 바인딩 (인자로 받은 컴포넌트 사용)
	const UInputAction* InventoryAction = InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Inventory);
    if (InventoryAction)
    {
        PlayerInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &UAZPlayerInventoryComponent::InputToggleInventory);
    }
}

void UAZPlayerInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAZPlayerInventoryComponent::InputToggleInventory(const struct FInputActionValue& Value)
{
	ToggleInventory();
}

void UAZPlayerInventoryComponent::ToggleInventory()
{
	AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
	if (PC == nullptr)
		return;
	if (!PC->IsLocalController())
		return;

	if (PC->InventoryWidget == nullptr)
		return;

	if (PC->InventoryWidget->GetVisibility() == ESlateVisibility::Visible) //플레이어 인벤토리UI가 켜져있을 때
	{
		FInputModeGameOnly InputModeGameOnly;
		PC->SetInputMode(InputModeGameOnly);
		PC->InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		PC->bShowMouseCursor = false;
		if (PC->InventoryWidget->LootInventoryGrid->GetVisibility() == ESlateVisibility::Visible)
		{
			PC->InventoryWidget->LootInventoryGrid->ItemsCanvasPanel->ClearChildren();
			PC->InventoryWidget->LootInventoryGrid->SetVisibility(ESlateVisibility::Collapsed); //상자 인벤토리를 숨기기
		}

		if (PC->InventoryWidget->SelectedInventoryVerticalBox->GetVisibility() == ESlateVisibility::Visible)
		{
			PC->InventoryWidget->BagInventoryGrid->GridCanvasPanel->ClearChildren();
			PC->InventoryWidget->BagInventoryGrid->ItemsCanvasPanel->ClearChildren();
			PC->InventoryWidget->BagInventoryGrid->SetVisibility(ESlateVisibility::Collapsed);
			PC->InventoryWidget->SelectedInventoryVerticalBox->SetVisibility(ESlateVisibility::Collapsed);
		}
		PC->CloseLootBox_Server();

		AAZHUD* HUD = Cast<AAZHUD>(PC->GetHUD());
		if (HUD)
		{
			HUD->HideTooltip();
		}
	}
	else if (PC->InventoryWidget->GetVisibility() == ESlateVisibility::Collapsed)
	{
		FInputModeGameAndUI InputModeGameAndUI;
		PC->InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		InputModeGameAndUI.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputModeGameAndUI);
		PC->bShowMouseCursor = true;
		PC->InventoryWidget->EquipmnetOverlay->SetVisibility(ESlateVisibility::Visible);

	}
}

void UAZPlayerInventoryComponent::OpenLobbyInventory()
{
	AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
	if (PC == nullptr)
		return;
	if (!PC->IsLocalController())
		return;

	if (PC->InventoryWidget == nullptr)
		return;

	PC->InventoryWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UAZPlayerInventoryComponent::UpdateBagImage()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;
	AAZPlayerController* PC = Cast<AAZPlayerController>(GetOwner());
	if (PC == nullptr)
		return;
	if (PC->EquipmentComp->Backpack)
	{
		UMaterialInstance* MI = DataManger->GetMaterialByID(PC->EquipmentComp->Backpack->GetItemID());
		if (MI && PC->InventoryWidget && PC->InventoryWidget->PlayerBagSlot && PC->InventoryWidget->PlayerBagSlot->ItemImage)
		{
			PC->InventoryWidget->PlayerBagSlot->ItemImage->SetBrushFromMaterial(MI);
		}
	}
	
}

int32 UAZPlayerInventoryComponent::GetItemCount(int32 ItemID)
{
	TArray<UAZItemBase*> ItemArray;
	GetAllItems().GetKeys(ItemArray);
	int32 TotalCount = 0;
	for (UAZItemBase* Item : ItemArray)
	{
		if (Item->GetItemID() == ItemID)
		{
			TotalCount += Item->GetStackCount();
		}
	}
	return TotalCount;
}