// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Subsystems/AZStoreSubsystem.h"
#include "UI/Store/AZStoreWidget.h"
#include "System/Player/AZPlayerController.h"  
#include "Components/AZStashComponent.h"       
#include "UI/Inventory/AZInventoryGridWidget.h"
#include "UI/Inventory/AZInventoryWidget.h"    
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "System/Subsystems/UAZCurrencySubsystem.h"
#include "DataTable/AZStoreDataTableRow.h"
#include "System/AZDataManagerSubsystem.h"
#include "Item/AZItemBase.h"
#include "DataTable/AZBaseItemDataTable.h"

bool UAZStoreSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer))
    {
        return false;
    }

    UWorld* World = Cast<UWorld>(Outer);
    if (!World)
    {
        return false;
    }

    FString MapName = World->GetMapName();
    if (MapName.Contains(TEXT("Lobbylevel_Hub")))
    {
        return true;
    }

	return false;
}

void UAZStoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UAZStoreSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UAZStoreSubsystem::InitStoreWidget(TSubclassOf<UAZStoreWidget> WidgetClass)
{
    if (!WidgetClass || !GetWorld()) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC == nullptr || !PC->IsLocalController()) return;

    StoreWidget = CreateWidget<UAZStoreWidget>(PC, WidgetClass);
    if (StoreWidget == nullptr) return;
    StoreWidget->AddToViewport(true);
    StoreWidget->SetVisibility(ESlateVisibility::Collapsed);
    StoreWidget->UpdateStoreList();
}

void UAZStoreSubsystem::ShowWidget()
{
    if (StoreWidget == nullptr) return;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC == nullptr) return;
    AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC);
    if (AZPC == nullptr) return;

    AZPC->SetShowMouseCursor(true);
    FInputModeUIOnly GameInputMode;
    AZPC->SetInputMode(GameInputMode);

    if (UAZStashComponent* StashComp = AZPC->StashComp)
    {
        StoreWidget->StashInventoryGrid->Init(StashComp);
        StoreWidget->StashInventoryGrid->CreateInvnetorySlot();
        StoreWidget->StashInventoryGrid->CreateAllItemWidget(StashComp);
        StashComp->InventoryGridWidget = StoreWidget->StashInventoryGrid;
    }

    StoreWidget->SetVisibility(ESlateVisibility::Visible);

}

void UAZStoreSubsystem::CloseWidget()
{
    if (StoreWidget == nullptr) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC == nullptr) return;
    AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC);
    if (AZPC == nullptr) return;

    if (AZPC)
    {
        if (UAZStashComponent* StashComp = AZPC->StashComp)
        {
            if (AZPC->InventoryWidget && AZPC->InventoryWidget->StashInventoryGrid)
            {
                StashComp->InventoryGridWidget = AZPC->InventoryWidget->StashInventoryGrid;
            }
        }
    }

    AZPC->SetShowMouseCursor(false);
    FInputModeGameOnly GameInputMode;
    AZPC->SetInputMode(GameInputMode);
    StoreWidget->SetVisibility(ESlateVisibility::Collapsed);
}
bool UAZStoreSubsystem::BuyItem(int32 ItemID, int32 Quantity)
{
    if (Quantity <= 0) return false;

    UWorld* World = GetWorld();
    if (World == nullptr) return false;

    UGameInstance* GI = World->GetGameInstance();
    if (GI == nullptr) return false;

    UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>();
    UUAZCurrencySubsystem* CurrencySys = GI->GetSubsystem<UUAZCurrencySubsystem>();
    AAZPlayerController* PC = Cast<AAZPlayerController>(World->GetFirstPlayerController());

    if (!DataManager || !CurrencySys || !PC || !PC->StashComp) return false;
    const FAZBaseItemDataTable* ItemaDataRow = DataManager->GetItemDataByID(ItemID);
    const TMap<int32, FAZStoreDataTableRow*>& StoreDataMap = DataManager->GetStoreDatas();
    if (!StoreDataMap.Contains(ItemID) || !ItemaDataRow) return false;

    FAZStoreDataTableRow* ItemRow = StoreDataMap[ItemID];
    int32 CurrentQuantity = Quantity;
    int32 UnitPrice = ItemRow->PurchasePrice;
    int32 TotalCost = UnitPrice * CurrentQuantity;

    int32 CurrentMoney = CurrencySys->GetMoney();
    if (CurrentMoney < UnitPrice) return false;

    int32 MaxStack = ItemaDataRow->MaxStack;
    int32 FinalCost = 0;
    // 아이템 생성
    while (CurrentQuantity > 0)
    {
        UAZItemBase* NewItem = NewObject<UAZItemBase>(PC->StashComp);
        if (!NewItem) return false;

        int32 AddCount = 0;
        if (MaxStack <= CurrentQuantity)
            AddCount = MaxStack;
        else
            AddCount = CurrentQuantity;

        NewItem->SetItemData(ItemID);
        NewItem->SetStackCount(AddCount);
        
        if (!PC->StashComp->TryAddItem(NewItem))
        {
            NewItem->ConditionalBeginDestroy();
            break;
        }
        CurrentQuantity -= AddCount;
        FinalCost += AddCount * UnitPrice; 
    }
   
    CurrencySys->SpendMoney(FinalCost);
    PC->StashComp->OnRep_Items();
    return true;
}

void UAZStoreSubsystem::SellItem(int32 ItemID, int32 Quantity)
{
    UWorld* World = GetWorld();
    if (!World) return;
    UGameInstance* GI = World->GetGameInstance();
    if (!GI) return;

    UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>();
    UUAZCurrencySubsystem* CurrencySys = GI->GetSubsystem<UUAZCurrencySubsystem>();
    AAZPlayerController* PC = Cast<AAZPlayerController>(World->GetFirstPlayerController());

    if (!DataManager || !CurrencySys || !PC || !PC->StashComp) return;

    const TMap<int32, FAZStoreDataTableRow*>& StoreDataMap = DataManager->GetStoreDatas();
    if (!StoreDataMap.Contains(ItemID)) return;

    FAZStoreDataTableRow* ItemData = StoreDataMap[ItemID];
    int32 UnitPrice = ItemData->BaseValue;

    int32 RemovedCount = PC->StashComp->RemoveItemByID(ItemID, Quantity);

    if (RemovedCount > 0)
    {
        int32 TotalPayout = RemovedCount * UnitPrice;
        CurrencySys->AddMoney(TotalPayout);
    }
    PC->StashComp->OnRep_Items();
}

void UAZStoreSubsystem::RemoveWidget()
{
    if (StoreWidget)
    {
        StoreWidget->RemoveFromParent();
        StoreWidget = nullptr;
    }
}