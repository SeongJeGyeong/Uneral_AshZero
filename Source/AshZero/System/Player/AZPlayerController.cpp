// Fill out your copyright notice in the Description page of Project Settings.


#include "AZPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/Inventory/AZInventoryWidget.h"
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "Components/AZInventoryComponent.h"
#include "Util/AZDefine.h"
#include "DataAsset/AZBagShapeDataAsset.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Item/AZItemBase.h"
#include "Components/AZPlayerInventoryComponent.h"
#include "Components/AZInteractionUIComponent.h"
#include "UI/Inventory/AZItemWidget.h"
#include "Components/AZDialogUIComponent.h"
#include "Components/AZSessionUIComponent.h"
#include "Components/AZLootBoxComponent.h"
#include "Interactables/AZChest.h"
#include "UI/DialogBox/AZDialogBoxUI.h"
#include "UI/Level/Lobby/AZLobbyUI.h"
#include "UI/Inventory/AZBagSlotWidget.h"
#include "Net/UnrealNetwork.h"
#include "Components/AZBagInventoryComponent.h"
#include "Item/AZBagItem.h"
#include "UI/Inventory/AZEquipmentSlot.h"
#include "Character/AZPlayerCharacter.h"
#include "UI/HUD/AZStatusBarWidget.h"
#include "Components/AZItemUsageComponent.h"
#include "UI/Crafting/AZCraftingWidget.h"
#include "Components/AZStashComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AZPlayerFireComponent.h"
#include "UI/HUD/AZCrosshairWidget.h"
#include "System/AZSessionSubsystem.h"
#include "Components/AZEquipmentComponent.h"
#include "System/GameMode/AZStageGameMode.h"
#include "Levels/AZRandomMapGenerator.h"
#include "UI/Level/Lobby/AZPartyUI.h"
#include "DataTable/AZWeaponItemDataTableRow.h"
#include "Weapon/AZWeapon.h"
#include "System/AZPlayerState.h"
#include "GameState/AZLobbyGameState.h"
#include "System/Subsystems/AZStorageSubsystem.h"
#include "System/Subsystems/AZSceneSubsystem.h"
#include "System/Subsystems/AZStoreSubsystem.h"
#include "UI/HUD/AZHUD.h"
#include "UI/HUD/AZExitUI.h"
#include "Components/AZHealthComponent.h"
#include "System/Subsystems/AZStoreSubsystem.h"

AAZPlayerController::AAZPlayerController()
{
	InventoryComp = CreateDefaultSubobject<UAZPlayerInventoryComponent>(TEXT("PlayerInventoryComp"));
	InventoryComp->SetIsReplicated(true);
	EquipmentComp = CreateDefaultSubobject<UAZEquipmentComponent>(TEXT("EquipmentComponent"));
	StashComp = CreateDefaultSubobject<UAZStashComponent>(TEXT("StashComp"));

	ItemUsageComp = CreateDefaultSubobject<UAZItemUsageComponent>(TEXT("ItemUsageComp"));

	InteractionUIComp = CreateDefaultSubobject<UAZInteractionUIComponent>(TEXT("Interaction UI Comp"));
	DialogUIComp = CreateDefaultSubobject<UAZDialogUIComponent>(TEXT("DialogBox UI Comp"));
	SessionUIComp = CreateDefaultSubobject<UAZSessionUIComponent>(TEXT("Session UI Comp"));

	ConstructorHelpers::FClassFinder<UUserWidget> SpectateWidget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/Hud/WBP_SpectateUI.WBP_SpectateUI_C'"));
	if (SpectateWidget.Succeeded())
	{
		SpectateWidgetClass = SpectateWidget.Class;
	}
}

void AAZPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAZPlayerController, InventoryComp);
	DOREPLIFETIME(AAZPlayerController, EquipmentComp);
	DOREPLIFETIME(AAZPlayerController, StashComp);
	DOREPLIFETIME(AAZPlayerController, TargetPawn);
	DOREPLIFETIME(AAZPlayerController, bIsSaveLoaded);
}

void AAZPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		//GetWorldTimerManager().SetTimerForNextTick(this, &AAZPlayerController::BindUIVisibilityRule);
		if (UAZSessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UAZSessionSubsystem>())
		{
			IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
			if (OSS && OSS->GetSubsystemName() == "STEAM")
			{
				IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
				if (Identity.IsValid())
				{
					SessionSubsystem->SteamId = Identity->GetPlayerNickname(0);
				}
			}
		}

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UAZStorageSubsystem* StorageSubsystem = GI->GetSubsystem<UAZStorageSubsystem>())
			{
				StorageSubsystem->LoadPlayerData(this);
			}
		}

		if (UWorld* World = GetWorld())
		{
			if (UAZStoreSubsystem* StoreSubsystem = World->GetSubsystem<UAZStoreSubsystem>())
			{
				StoreSubsystem->InitStoreWidget(StoreWidgetClass);
			}
		}
		
	}

	if (CraftingWidgetClass != nullptr)
	{
		CraftingWidget = CreateWidget<UAZCraftingWidget>(GetWorld(), CraftingWidgetClass);
		if (CraftingWidget == nullptr)
			return;
		CraftingWidget->AddToViewport(5);
		CraftingWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}

void AAZPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//if (IsLocalController())
	//{
	//	if (EndPlayReason == EEndPlayReason::LevelTransition ||
	//		EndPlayReason == EEndPlayReason::Destroyed ||
	//		EndPlayReason == EEndPlayReason::EndPlayInEditor)
	//	{
	//		if (UGameInstance* GI = GetGameInstance())
	//		{
	//			if (UAZStorageSubsystem* StorageSubsystem = GI->GetSubsystem<UAZStorageSubsystem>())
	//			{
	//				StorageSubsystem->SavePlayerData(this);
	//			}
	//		}
	//	}
	//}
}

void AAZPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsLocalController() && SessionUIComp)
	{
		// SessionUIComp가 UI를 가지고 있다면 갱신을 요청
		if (SessionUIComp->PartyUI)
		{
			SessionUIComp->PartyUI->UpdatePartyUI();
		}
	}
}


void AAZPlayerController::ClientRestart_Implementation(APawn* NewPawn)
{
	Super::ClientRestart_Implementation(NewPawn);
	if (!IsLocalController())
	{
		return;
	}

	if (NewPawn && NewPawn->IsA<AAZPlayerCharacter>())
	{
		InventoryComp->InitPlayerInventoryComponent();
		InitializeStartingLoadout_Server();
		StashComp->OnRep_Items();
		EquipmentComp->RefreshSlots();
		if (AAZPlayerCharacter* MyPawn = Cast<AAZPlayerCharacter>(GetPawn()))
		{
			if (MyPawn->HealthComp)
			{
				MyPawn->HealthComp->SetHealth(MyPawn->HealthComp->GetMaxHp());
			}
		}
	}
}

//void AAZPlayerController::SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
//{
//	Super::SetViewTarget(NewViewTarget, TransitionParams);
//	if (AAZHUD* HUD = Cast<AAZHUD>(GetHUD()))
//	{
//		HUD->UpdateSpectatorTarget(NewViewTarget);
//	}
//}

void AAZPlayerController::ClearAllWidgets()
{
	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
		InventoryWidget = nullptr;
	}
	if (InventoryComp)
	{
		InventoryComp->bInitialized = false;
	}
}

void AAZPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(InputComponent);
	//if (InputComp)
	//{
	//	InputComp->BindAction(OpenSessionUIAction, ETriggerEvent::Triggered, SessionUIComp.Get(), &UAZSessionUIComponent::OpenSessionUI);
	//}
}


bool AAZPlayerController::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (InventoryComp != nullptr)
	{
		bWroteSomething |= InventoryComp->ReplicateSubobjectsFromItems(Channel, Bunch, RepFlags);
	}

	if (EquipmentComp != nullptr)
	{
		bWroteSomething |= EquipmentComp->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}

	if (StashComp != nullptr)
	{
		bWroteSomething |= StashComp->ReplicateSubobjectsFromItems(Channel, Bunch, RepFlags);
	}

	return bWroteSomething;
}

UAZInventoryWidget* AAZPlayerController::GetInventoryWidget()
{
	if (!IsLocalController())
	{
		return nullptr;
	}
	if (InventoryWidget)
	{
		return InventoryWidget;
	}

	if (InventoryWidgetClass == nullptr)
		return nullptr;

	InventoryWidget = CreateWidget<UAZInventoryWidget>(this, InventoryWidgetClass);

	if (InventoryWidget == nullptr)
		return nullptr;

	if (EquipmentComp == nullptr)
		return nullptr;
	
	InventoryWidget->AddToViewport(15);
	InventoryWidget->InitInventoryWidget(EquipmentComp);
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	InventoryWidget->LootInventoryGrid->SetVisibility(ESlateVisibility::Collapsed);
	InventoryWidget->BagInventoryGrid->SetVisibility(ESlateVisibility::Collapsed);
	//InventoryWidget->StashInventoryGrid->SetVisibility(ESlateVisibility::Collapsed);

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return nullptr;

	UAZSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UAZSessionSubsystem>();
	if (SessionSubsystem == nullptr)
		return nullptr;
	//SessionSubsystem->OnGameStateChanged.RemoveDynamic(this, &AAZPlayerController::OnGameStateChanged);
	//SessionSubsystem->OnGameStateChanged.AddDynamic(this, &AAZPlayerController::OnGameStateChanged);
	
	OnGameStateChanged(SessionSubsystem->CurrentGameState);

	InventoryWidget->PlayerBagSlot->InitBagSlot(InventoryComp);
	InventoryWidget->HelmetSlot->InitEquipmentSlot(InventoryComp, EquipmentComp);
	InventoryWidget->ArmorSlot->InitEquipmentSlot(InventoryComp, EquipmentComp);
	InventoryWidget->GlovesSlot->InitEquipmentSlot(InventoryComp, EquipmentComp);
	InventoryWidget->BootsSlot->InitEquipmentSlot(InventoryComp, EquipmentComp);
	InventoryWidget->FirstWeaponSlot->InitEquipmentSlot(InventoryComp, EquipmentComp);
	InventoryWidget->SecondWeaponSlot->InitEquipmentSlot(InventoryComp, EquipmentComp);
	InventoryWidget->QuickSlot_1->InitEquipmentSlot(InventoryComp, EquipmentComp);
	InventoryWidget->QuickSlot_2->InitEquipmentSlot(InventoryComp, EquipmentComp);
	InventoryWidget->QuickSlot_3->InitEquipmentSlot(InventoryComp, EquipmentComp);
	InventoryWidget->QuickSlot_4->InitEquipmentSlot(InventoryComp, EquipmentComp);

	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return nullptr;

	const FBagDefinition* BagShape = DataManger->GetBagShapeByName("Chest");
	if (BagShape)
	{
		InventoryWidget->LootInventoryGrid->CreateInvnetorySlotByData(*BagShape, InventoryComp->TileSize);
	}

	return InventoryWidget;
}

void AAZPlayerController::RequestOpenLootBox_Server_Implementation(UAZLootBoxComponent* LootBoxComponent, AAZChest* Chest)
{
	if (!Chest || !LootBoxComponent) return;

	if (CurrentOpenChest && CurrentOpenChest != Chest)
	{
		CurrentOpenChest->UnregisterInteractor(GetPawn());
	}

	CurrentOpenChest = Chest;
	CurrentOpenChest->RegisterInteractor(GetPawn());

	if (LootBoxComponent->bIsItemsGenerated == false)
	{
		LootBoxComponent->GenerateRandomItems();
		Chest->InitBagItems();
		Chest->SetIsItemGenerated_Server(true);
	}
}

void AAZPlayerController::CloseLootBox_Server_Implementation()
{
	if (CurrentOpenChest)
	{
		if (APawn* ControlledPawn = GetPawn())
		{
			CurrentOpenChest->UnregisterInteractor(ControlledPawn);
			CurrentOpenChest = nullptr;
		}
	}
}

void AAZPlayerController::MoveItem_Server_Implementation(UAZInventoryComponent* SourceComp, UAZInventoryComponent* DestComp, UAZItemBase* Item, int32 TopLeftIndex, bool bIsRotated)
{
	if (SourceComp && DestComp && Item)
	{
		Item->SetIsDragged(false);
		SourceComp->RemoveItem(Item);
		Item->SetIsRotated(bIsRotated);
		DestComp->AddItemAt(Item, TopLeftIndex);
		SourceComp->OnRep_Items();
		DestComp->OnRep_Items();
	}
}
void AAZPlayerController::SetItemDrag_Server_Implementation(UAZItemBase* Item, bool bIsDrag)
{
	if (Item)
	{
		Item->SetIsDragged(bIsDrag);
	}
}

void AAZPlayerController::TryAddItemToInventory_Server_Implementation(UAZInventoryComponent* InventoryComponent, UAZItemBase* Item)
{
	if (InventoryComponent && Item)
	{
		InventoryComponent->TryAddItem(Item);
		InventoryComponent->OnRep_Items();
	}
}

void AAZPlayerController::AddItemToInventory_Server_Implementation(UAZInventoryComponent* InventoryComponent, UAZItemBase* Item, int32 TopLeftIndex,bool bIsRotated)
{
	if (InventoryComponent && Item)
	{
		Item->SetIsRotated(bIsRotated);
		InventoryComponent->AddItemAt(Item, TopLeftIndex);
		InventoryComponent->OnRep_Items();
	}
}

void AAZPlayerController::RemoveItemToInventory_Server_Implementation(UAZInventoryComponent* InventoryComponent, UAZItemBase* Item)
{
	if (InventoryComponent && Item)
	{
		InventoryComponent->RemoveItem(Item);
		InventoryComponent->OnRep_Items();
	}
}

void AAZPlayerController::ReturnToMenu_Client_Implementation()
{
	IOnlineSubsystem* subsystem = Online::GetSubsystem(GetWorld());
	if (IsLocalController())
	{
		UGameInstance* GI = GetGameInstance();
		if (GI)
		{
			if (UAZStorageSubsystem* StorageSubsystem = GI->GetSubsystem<UAZStorageSubsystem>())
			{
				StorageSubsystem->SavePlayerData(this);
			}

			if (UAZSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UAZSessionSubsystem>())
			{
				SessionSubsystem->CurrentGameState = EGameState::Lobby;
			}

			if (UAZStorageSubsystem* StorageSubsystem = GI->GetSubsystem<UAZStorageSubsystem>())
			{
				AAZPlayerCharacter* AZChar = Cast<AAZPlayerCharacter>(GetPawn());
				if (AZChar && AZChar->HealthComp)
				{
					if (AZChar->HealthComp->Hp <= 0)
					{
						StorageSubsystem->ClearAllItems(this);
					}
					else if (IsInState(NAME_Spectating))
					{
						StorageSubsystem->ClearAllItems(this);
					}
				}
			}
		}

		
	}

	if (subsystem)
	{
		IOnlineSessionPtr sessionInterface = subsystem->GetSessionInterface();
		sessionInterface->DestroySession(NAME_GameSession);
	}
}



void AAZPlayerController::RefreshInventory_Server_Implementation(UAZInventoryComponent* InventoryComponent)
{
	InventoryComponent->OnRep_Items();
	RefreshInventory_Multicast(InventoryComponent);
}

void AAZPlayerController::RefreshInventory_Multicast_Implementation(UAZInventoryComponent* InventoryComponent)
{
	InventoryComponent->OnRep_Items();
}


void AAZPlayerController::BindUIVisibilityRule()
{
	if (!IsLocalController()) return;
	if (!DialogUIComp || !InteractionUIComp || !SessionUIComp) return;
	if (!DialogUIComp->DialogBoxWidget) return;
}

void AAZPlayerController::ChangeCameraViewTarget(AActor* newTarget)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float Elapsed = CurrentTime - BlendStartTime;
	float RemainTime = FMath::Min(CameraBlendTime, Elapsed);
	SetViewTargetWithBlend(newTarget, RemainTime, VTBlend_Linear, 0.f, true);
	BlendStartTime = CurrentTime;
}

void AAZPlayerController::TryStackItem_Server_Implementation(UAZInventoryComponent* SourceComponent, UAZInventoryComponent* DestComponent, UAZItemBase* SourceItem, UAZItemBase* DestItem, UAZItemWidget* SourceWidget, UAZItemWidget* DestWidget)
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;
	if (DestItem == nullptr)
		return;
	if (SourceItem == nullptr)
		return;

	if (SourceItem == DestItem)
	{
		SetItemDrag_Server(DestItem, false);
		return;
	}
	if (SourceItem->GetItemID() == DestItem->GetItemID())
	{
		int32 SourceItemStackcount = SourceItem->GetStackCount();
		int32 DestItemStackCount = DestItem->GetStackCount();
		int32 SumCount = SourceItemStackcount + DestItemStackCount;
		int32 MaxStackCount = DataManger->GetMaxStackSizeByID(DestItem->GetItemID());
		if (SumCount > MaxStackCount)	//�ִ� ���ú��� ���ٸ� 
		{
			SourceComponent->SetItemStack_Multicast(SourceItem, SumCount - MaxStackCount);
			DestComponent->SetItemStack_Multicast(DestItem, MaxStackCount);
		}
		else
		{
			SourceComponent->RemoveItem_Server(SourceItem);
			DestComponent->SetItemStack_Multicast(DestItem, SumCount);
			SourceComponent->OnRep_Items();
		}
	}
	SetItemDrag_Server(DestItem, false);
}

void AAZPlayerController::OnGameStateChanged(EGameState NewState)
{
	if (NewState == EGameState::Lobby)
	{
		StashComp->InitStashComponent();
	}

	if (InventoryWidget && InventoryWidget->IsValidLowLevel())
	{
		InventoryWidget->SetUIMode(NewState);
	}
}

void AAZPlayerController::PreTravelCleanUp_Client_Implementation()
{
	if (!IsLocalController())
		return;
	ClearAllWidgets();
	//InventoryComp->ResetInventoryData_Server();
}

void AAZPlayerController::SetHUDVisibility(ESlateVisibility Visibility)
{
	AAZPlayerCharacter* PlayerCharacter = Cast<AAZPlayerCharacter>(GetCharacter());
	if (!PlayerCharacter || !PlayerCharacter->StatusBarWidget) return;

	PlayerCharacter->StatusBarWidget->SetVisibility(Visibility);
	PlayerCharacter->FireComp->CrosshairWidget->SetVisibility(Visibility);
}

void AAZPlayerController::SetMouseMode(bool bShow)
{
	if (bShow)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAZPlayerController::SetMouseMode UIOnly: %d"), GetUniqueID());
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		SetShowMouseCursor(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AAZPlayerController::SetMouseMode GameOnly: %d"), GetUniqueID());
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		SetShowMouseCursor(false);
	}
}

void AAZPlayerController::SetPartyUI()
{
	if (!IsLocalController()) return;
	if (SessionUIComp)
	{
		SessionUIComp->ConstructUI();
	}
}

void AAZPlayerController::HandleDeathFinished()
{
	if (!HasAuthority()) return;

	AAZStageGameMode* GameMode = GetWorld()->GetAuthGameMode<AAZStageGameMode>();

	if (!GameMode) return;

	GameMode->HandlePlayerReadyToSpectate(this);
}

void AAZPlayerController::ReturnToLobby()
{
	if (!HasAuthority()) return;
	//AAZPlayerCharacter* AZChar = Cast<AAZPlayerCharacter>(GetPawn());
	//if (AZChar && AZChar->HealthComp)
	//{
	//	if (AZChar->HealthComp->Hp <= 0)
	//	{
	//		StorageSubsystem->ClearAllItems(this);
	//	}
	//}
	GetWorld()->ServerTravel(TEXT("/Game/Maps/LobbyLevel_Party?listen"), true);
}

void AAZPlayerController::OnRep_SpectateTarget()
{
	if (TargetPawn)
	{
		SetViewTarget(TargetPawn);
	}
}

void AAZPlayerController::EnterRoadOut()
{
	if (InventoryWidget && InventoryWidget->IsValidLowLevel())
	{
		//InventoryWidget->SetUIMode(EGameState::Lobby);
		InventoryComp->OpenLobbyInventory();
		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AAZPlayerController::SwitchPartyUI()
{
	if (SessionUIComp)
	{
		SessionUIComp->PartyUI->SetVisibilityPartyUI(true);
	}
}

void AAZPlayerController::InitializeStartingLoadout_Server_Implementation()
{
	if (EquipmentComp == nullptr) return;

	UAZItemBase* StarterItem = NewObject<UAZItemBase>(this);
	StarterItem->SetItemID(401);
	StarterItem->bIsStarterItem = true;

	EquipmentComp->FirstWeapon = StarterItem;
	EquipmentComp->HandleItemRepNotify(EEquipmentSlot::Weapon, ESlotIndex::Slot_0, StarterItem);
	SpawnWeaponActor(StarterItem, ESlotIndex::Slot_0);

	UAZItemBase* LoadoutItem = EquipmentComp->SecondWeapon;
	if (LoadoutItem != nullptr && LoadoutItem->GetItemID() != 0)
	{
		SpawnWeaponActor(LoadoutItem, ESlotIndex::Slot_1);
	}
}

void AAZPlayerController::SpawnWeaponActor(UAZItemBase* Item, ESlotIndex SlotIndex)
{
	if (Item == nullptr) return;

	AAZPlayerCharacter* PlayerCharacter = Cast<AAZPlayerCharacter>(GetPawn());
	if (PlayerCharacter == nullptr) return;

	UAZPlayerFireComponent* FireComponent = PlayerCharacter->FireComp;
	if (FireComponent == nullptr) return;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr) return;

	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr) return;

	const FAZWeaponItemDataTableRow* WeaponData = DataManger->GetItemDataByID<FAZWeaponItemDataTableRow>(Item->GetItemID());

	if (WeaponData && WeaponData->WeaponActorClass)
	{
		FireComponent->EquipWeaponToSlot(WeaponData->WeaponActorClass, int32(SlotIndex), 0);
	}
}

void AAZPlayerController::StartMapGenerate()
{
	UAZSessionSubsystem* SessionSystem = GetGameInstance()->GetSubsystem<UAZSessionSubsystem>();
	if (!SessionSystem) return;

	AAZRandomMapGenerator* MapGenerator = Cast<AAZRandomMapGenerator>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AAZRandomMapGenerator::StaticClass())
	);

	if (MapGenerator)
	{
		// 서버에서 값을 넣으면 -> OnRep을 통해 클라이언트들도 자동 실행됨
		MapGenerator->SetSeedAndGenerate(SessionSystem->RandomSeed);
	}
}

void AAZPlayerController::OnMapGenerationFinished()
{
	if (IsLocalController())
	{
		NotifyMapLoadComplete_Server();
	}
}

void AAZPlayerController::NotifyMapLoadComplete_Server_Implementation()
{
	if (AAZStageGameMode* GM = GetWorld()->GetAuthGameMode<AAZStageGameMode>())
	{
		GM->HandlePlayerMapLoaded(this);
	}
}

void AAZPlayerController::HideLoadingScreen_Multicast_Implementation()
{
	UAZSceneSubsystem* SceneSystem = GetGameInstance()->GetSubsystem<UAZSceneSubsystem>();
	if (SceneSystem)
	{
		SceneSystem->HideLoadingScreen();
	}
}

void AAZPlayerController::SetSpectatorView_Client_Implementation(APawn* Target)
{
	if (!Target) return;

	UUserWidget* Widget = CreateWidget(this, SpectateWidgetClass);
	if(Widget) Widget->AddToViewport(1);
	SetViewTarget(Target);

	if (AAZHUD* HUD = Cast<AAZHUD>(GetHUD()))
		HUD->UpdateSpectatorTarget(Target);
}

void AAZPlayerController::SetReady_Server_Implementation(bool bReady)
{
	if (AAZPlayerState* PS = GetPlayerState<AAZPlayerState>())
	{
		PS->SetReady(bReady);

		if (AAZLobbyGameState* GS = GetWorld()->GetGameState<AAZLobbyGameState>())
		{
			GS->PartyUpdated_Server();
		}
	}
}

void AAZPlayerController::SyncPlayerData_Server_Implementation(const FAZPlayerSaveData& SaveData)
{
	ApplySaveData(SaveData);
}

void AAZPlayerController::ApplySaveData(const FAZPlayerSaveData& SaveData)
{
	UGameInstance* GI = GetGameInstance();
	if (GI == nullptr) return;
	UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManager == nullptr) return;

	//장비 아이템 장착
	if (EquipmentComp)
	{
		EquipmentComp->Helmet = nullptr;
		EquipmentComp->Armor = nullptr;
		EquipmentComp->Gloves = nullptr;
		EquipmentComp->Boots = nullptr;
		EquipmentComp->Backpack = nullptr;
		EquipmentComp->SecondWeapon = nullptr;

		for (const auto& Pair : SaveData.EquipmentItems)
		{
			EEquipmentSlot SlotType = Pair.Slot;
			const FAZItemSaveData& ItemData = Pair.ItemData;

			UAZItemBase* NewItem = CreateItem(ItemData, DataManager);
			ESlotIndex SlotIndex = ESlotIndex::Slot_0;
			if (NewItem)
			{
				// EquipmentComponent에 적절한 변수에 할당
				switch (SlotType)
				{
				case EEquipmentSlot::Helmet:	EquipmentComp->Helmet = NewItem;		break;
				case EEquipmentSlot::Armor:		EquipmentComp->Armor = NewItem;			break;
				case EEquipmentSlot::Gloves:	EquipmentComp->Gloves = NewItem;		break;
				case EEquipmentSlot::Boots:		EquipmentComp->Boots = NewItem;			break;
				case EEquipmentSlot::Backpack:	
				{
					UAZBagItem* BagItem = Cast<UAZBagItem>(NewItem);
					if (BagItem && InventoryComp)
					{
						BagItem->SetItemData(ItemData.ItemId);
						EquipmentComp->Backpack = BagItem;
						InventoryComp->InventoryStruct = ItemData.BagStruct;
						InventoryComp->Items.SetNum(InventoryComp->InventoryStruct.Columns * InventoryComp->InventoryStruct.Rows);
						InventoryComp->OnRep_InventoryStruct();
						InventoryComp->OnRep_Items();
						InventoryComp->UpdateBagImage();
					}
					break;
				}
				case EEquipmentSlot::Weapon:
					EquipmentComp->SecondWeapon = NewItem;
					SlotIndex = ESlotIndex::Slot_1;
					break;
				default: break;
				}
			}
			EquipmentComp->HandleItemRepNotify(SlotType, SlotIndex, NewItem);
		}

		EquipmentComp->QuickSlotItem_1 = nullptr;
		EquipmentComp->QuickSlotItem_2 = nullptr;
		EquipmentComp->QuickSlotItem_3 = nullptr;
		EquipmentComp->QuickSlotItem_4 = nullptr;

		for (const auto& Pair : SaveData.QuickSlotItems)
		{
			ESlotIndex SlotIndex = ESlotIndex(Pair.SlotIndex);
			const FAZItemSaveData& ItemData = Pair.ItemData;
			UAZItemBase* NewItem = CreateItem(ItemData, DataManager);
			if (NewItem)
			{
				switch (SlotIndex)
				{
				case ESlotIndex::Slot_1: EquipmentComp->QuickSlotItem_1 = NewItem; break;
				case ESlotIndex::Slot_2: EquipmentComp->QuickSlotItem_2 = NewItem; break;
				case ESlotIndex::Slot_3: EquipmentComp->QuickSlotItem_3 = NewItem; break;
				case ESlotIndex::Slot_4: EquipmentComp->QuickSlotItem_4 = NewItem; break;
				}
			}
			EquipmentComp->HandleItemRepNotify(EEquipmentSlot::QuickSlot, SlotIndex, NewItem);
		}
		EquipmentComp->RefreshSlots();
		EquipmentComp->UpdateReplicatedStats_Server();
	}

	if (InventoryComp)
	{
		InventoryComp->AllItems.Empty(); // 기존 아이템 제거
		for (const FAZItemSaveData& ItemData : SaveData.InventoryItems)
		{
			UAZItemBase* NewItem = CreateItem(ItemData, DataManager);
			if (NewItem)
			{
				UAZBagItem* BagItem = Cast<UAZBagItem>(NewItem);
				if (BagItem != nullptr && BagItem->InventoryComponent != nullptr)
				{
					BagItem->InventoryComponent->InventoryStruct = SaveData.PlayerIventoryStruct;
				}
				int32 Index = InventoryComp->TileToIndex(ItemData.Index);
				InventoryComp->AddItemAt(NewItem, Index);
			}
		}
		InventoryComp->OnRep_Items();
		InventoryComp->OnRep_InventoryStruct();
	}

	if (StashComp)
	{
		if (SaveData.StashInventoryStruct.BagName.ToString() == "New Bag")
		{
			return;
		}
		StashComp->AllItems.Empty();
		StashComp->InventoryStruct = SaveData.StashInventoryStruct;
		StashComp->Items.SetNum(StashComp->InventoryStruct.Columns * StashComp->InventoryStruct.Rows);
		for (const FAZItemSaveData& ItemData : SaveData.StashItems)
		{
			UAZItemBase* NewItem = CreateItem(ItemData, DataManager);
			if (NewItem)
			{
				UAZBagItem* BagItem = Cast<UAZBagItem>(NewItem);
				if (BagItem != nullptr && BagItem->InventoryComponent != nullptr)
				{
					BagItem->InventoryComponent->InventoryStruct = SaveData.PlayerIventoryStruct;
				}
				int32 Index = StashComp->TileToIndex(ItemData.Index);
				StashComp->AddItemAt(NewItem, Index);
			}
		}
		StashComp->OnRep_Items();
		StashComp->OnRep_InventoryStruct();
	}

	bIsSaveLoaded = true;
	if (IsLocalController())
	{
		if (EquipmentComp)
			EquipmentComp->RefreshSlots();
	}

}

void AAZPlayerController::CreateWarpWidget()
{
	if (WarpWidget || !WarpWidgetClass)
	{
		return;
	}

	WarpWidget = CreateWidget<UAZExitUI>(this, WarpWidgetClass);
	if (WarpWidget)
	{
		WarpWidget->AddToViewport(30);
		//WarpWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AAZPlayerController::RemoveWarpWidget()
{
	WarpWidget->RemoveFromParent();
}

void AAZPlayerController::ShowWarpUI(bool bShow)
{
	if (WarpWidget)
	{
		WarpWidget->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void AAZPlayerController::UpdateWarpProgress(float Progress)
{
	if (WarpWidget)
	{
		WarpWidget->SetPercent(Progress);
	}
}

void AAZPlayerController::OnRep_IsSaveLoaded()
{
	if (bIsSaveLoaded)
	{
		if (EquipmentComp)
		{
			EquipmentComp->RefreshSlots();
		}
		if (InventoryComp)
		{
			InventoryComp->OnRep_Items();
		}
		if (StashComp)
		{
			StashComp->OnRep_Items();
		}
	}
}

UAZItemBase* AAZPlayerController::CreateItem(const FAZItemSaveData& ItemData, UAZDataManagerSubsystem* DataManager)
{
	if (!DataManager) return nullptr;

	if (ItemData.ItemId == 0 || ItemData.Count <= 0)
	{
		return nullptr;
	}

	TSubclassOf<UAZItemBase> ItemClass = UAZItemBase::StaticClass();
	if (ItemData.bIsBagItem)
	{
		ItemClass = UAZBagItem::StaticClass();
	}

	UAZItemBase* NewItem = NewObject<UAZItemBase>(this, ItemClass);

	if (NewItem)
	{
		NewItem->SetItemID(ItemData.ItemId);
		NewItem->SetStackCount(ItemData.Count);

		// 가방 데이터 복원
		if (ItemData.bIsBagItem)
		{
			UAZBagItem* BagItem = Cast<UAZBagItem>(NewItem);
			if (BagItem)
			{
				BagItem->SetBagData(ItemData.BagStruct);
				UAZBagInventoryComponent* NewBagInventoryComp = NewObject<UAZBagInventoryComponent>(this);

				if (NewBagInventoryComp)
				{
					NewBagInventoryComp->CreationMethod = EComponentCreationMethod::UserConstructionScript;
					NewBagInventoryComp->SetIsReplicated(true);
					NewBagInventoryComp->RegisterComponent();
					BagItem->InitBagItem(NewBagInventoryComp);
				}

			}
		}
	}

	return NewItem;
}
