// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Store/AZStoreWidget.h"
#include "System/Subsystems/AZStoreSubsystem.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "DataTable/AZStoreDataTableRow.h"
#include "UI/Store/AZStoreItemSlotWidget.h"
#include "System/AZDataManagerSubsystem.h"
#include "UI/Store/AZQuantitySelectorWidget.h"
#include "System/Subsystems/UAZCurrencySubsystem.h"
#include "System/Player/AZPlayerController.h"
#include "Components/AZStashComponent.h"
#include "UI/Operation/AZDragDropOperation.h"
#include "Item/AZItemBase.h"
#include "Blueprint/DragDropOperation.h"


void UAZStoreWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UAZStoreWidget::OnCloseButtonClicked);
    }

    if (QuantitySelectorClass)
    {
        QuantitySelectorInstance = CreateWidget<UAZQuantitySelectorWidget>(this, QuantitySelectorClass);
        if (QuantitySelectorInstance)
        {
            QuantitySelectorInstance->AddToViewport(15);
            QuantitySelectorInstance->SetVisibility(ESlateVisibility::Collapsed);

            QuantitySelectorInstance->OnConfirm.AddDynamic(this, &UAZStoreWidget::OnQuantityConfirmed);
        }
    }

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UUAZCurrencySubsystem* CurrencySys = GI->GetSubsystem<UUAZCurrencySubsystem>())
            {
                CurrencySys->OnMoneyChanged.AddDynamic(this, &UAZStoreWidget::OnCurrencyChanged);
                OnCurrencyChanged(CurrencySys->GetMoney());
            }
        }
    }
}

bool UAZStoreWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UAZDragDropOperation* DragDropOp = Cast<UAZDragDropOperation>(InOperation);

    if (DragDropOp && DragDropOp->Payload)
    {
        if (StoreListScroll && StoreListScroll->GetCachedGeometry().IsUnderLocation(InDragDropEvent.GetScreenSpacePosition()))
        {
            UAZItemBase* DroppedItem = Cast<UAZItemBase>(DragDropOp->Payload);
            if (DroppedItem)
            {
                DroppedItem->SetIsDragged(false);
                int32 ItemID = DroppedItem->GetItemID();

                OnStoreSlotSelected(ItemID, EAZTradeMode::Sell);
                return true;
            }  
        }
    }

    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);;
}

void UAZStoreWidget::UpdateStoreList()
{
    if (!StoreListScroll || !ItemSlotWidgetClass) return;
    StoreListScroll->ClearChildren();

    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (GI == nullptr) return;
    UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>();
    if (DataManager == nullptr) return;

    TMap<int32, FAZStoreDataTableRow*> AllRows = DataManager->GetStoreDatas();
    if (AllRows.Num() == 0) return;

    for (const auto Row : AllRows)
    {
        if (Row.Value == nullptr) continue;
        if (Row.Value->PurchasePrice <= 0) continue;
        UAZStoreItemSlotWidget* NewSlot = CreateWidget<UAZStoreItemSlotWidget>(this, ItemSlotWidgetClass);
        if (NewSlot == nullptr) continue;
        NewSlot->InitSlot(Row.Value);
        StoreListScroll->AddChild(NewSlot);
    }
}

void UAZStoreWidget::OnQuantityConfirmed(int32 TargetID, int32 Quantity, EAZTradeMode TradeMode)
{
    if (UWorld* World = GetWorld())
    {
        if (UAZStoreSubsystem* StoreSubsystem = World->GetSubsystem<UAZStoreSubsystem>())
        {
            if (TradeMode == EAZTradeMode::Buy)
            {
                bool BoughtCount = StoreSubsystem->BuyItem(TargetID, Quantity);

                if (BoughtCount)
                {
                    // 구매 성공
                }
                else
                {
                    // 구매 실패
                }
            }
            else if (TradeMode == EAZTradeMode::Sell)
            {
                StoreSubsystem->SellItem(TargetID, Quantity);
            }
        }
    }
}

void UAZStoreWidget::OnCloseButtonClicked()
{
    if (UWorld* World = GetWorld())
    {
        if (UAZStoreSubsystem* StoreSubsystem = World->GetSubsystem<UAZStoreSubsystem>())
        {
            StoreSubsystem->CloseWidget();
        }
    }
}

void UAZStoreWidget::OnCurrencyChanged(int32 NewAmount)
{
    if (CurrencyText == nullptr) return;
    CurrencyText->SetText(FText::AsNumber(NewAmount));
}

void UAZStoreWidget::OnStoreSlotSelected(int32 TargetID, EAZTradeMode TradeMode)
{
    if (QuantitySelectorInstance == nullptr) return;

    if (QuantitySelectorInstance == nullptr) return;

    UWorld* World = GetWorld();
    if (World == nullptr) return;

    UGameInstance* GI = World->GetGameInstance();
    if (GI == nullptr) return;

    UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>();
    UUAZCurrencySubsystem* CurrencySys = GI->GetSubsystem<UUAZCurrencySubsystem>();
    AAZPlayerController* PC = Cast<AAZPlayerController>(World->GetFirstPlayerController());

    if (!DataManager || !CurrencySys || !PC || !PC->StashComp) return;

    const TMap<int32, FAZStoreDataTableRow*>& StoreDataMap = DataManager->GetStoreDatas();
    if (!StoreDataMap.Contains(TargetID)) return;

    FAZStoreDataTableRow* ItemData = StoreDataMap[TargetID];
    if (!ItemData) return;
    
    int32 MaxCount = 0;
    int32 UnitPrice = 0;

    if(TradeMode == EAZTradeMode::Buy)
    {
        int32 MyMoney = CurrencySys->GetMoney();
        UnitPrice = ItemData->PurchasePrice;

        if (UnitPrice > 0)
        {
            MaxCount = MyMoney / UnitPrice;
        }
    }
    else
    {
        UnitPrice = ItemData->BaseValue;
        MaxCount = PC->StashComp->GetTotalItemCount(TargetID);
    }

    QuantitySelectorInstance->InitSelector(TargetID, MaxCount, TradeMode);
    QuantitySelectorInstance->SetVisibility(ESlateVisibility::Visible);
}
