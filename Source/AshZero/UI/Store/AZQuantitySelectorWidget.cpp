// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Store/AZQuantitySelectorWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "System/AZDataManagerSubsystem.h"
#include "DataTable/AZBaseItemDataTable.h"
#include "DataTable/AZStoreDataTableRow.h"
#include "Util/AZDefine.h"
#include "UI/HUD/AZHUD.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"

void UAZQuantitySelectorWidget::NativeConstruct()
{
	Super::NativeConstruct();

    if (PlusButton) PlusButton->OnClicked.AddDynamic(this, &UAZQuantitySelectorWidget::OnPlusClicked);
    if (MinusButton) MinusButton->OnClicked.AddDynamic(this, &UAZQuantitySelectorWidget::OnMinusClicked);
    if (ConfirmButton) ConfirmButton->OnClicked.AddDynamic(this, &UAZQuantitySelectorWidget::OnConfirmClicked);
    if (CancelButton) CancelButton->OnClicked.AddDynamic(this, &UAZQuantitySelectorWidget::OnCancelClicked);
    if (QuantityInputBox) QuantityInputBox->OnTextCommitted.AddDynamic(this, &UAZQuantitySelectorWidget::OnQuantityTextCommitted);
}

void UAZQuantitySelectorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!ItemImage || !GetOwningPlayer())
    {
        return;
    }

    FVector2D MousePosition = FSlateApplication::Get().GetCursorPos();
    bool bCurrentlyUnder = ItemImage->GetCachedGeometry().IsUnderLocation(MousePosition);

    AAZHUD* HUD = Cast<AAZHUD>(GetOwningPlayer()->GetHUD());
    if (!HUD)
    {
        return;
    }

    if (bCurrentlyUnder && !bIsHovering)
    {
        bIsHovering = true;
        HUD->ShowTooltipByID(ID);
    }
    else if (!bCurrentlyUnder && bIsHovering)
    {
        bIsHovering = false;
        HUD->HideTooltip();
    }

}

void UAZQuantitySelectorWidget::InitSelector(int32 TargetID, int32 NewMaxCount, EAZTradeMode TradeMode)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    UGameInstance* GI = World->GetGameInstance();
    if (GI == nullptr) return;
    UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
    if (DataManger == nullptr) return;

    const FAZBaseItemDataTable* ItemDataRow = DataManger->GetItemDataByID(TargetID);
    ID = TargetID;
    if (ItemDataRow == nullptr) return;
    if (ItemImage && ItemDataRow->MaterialAsset)
    {
        ItemImage->SetBrushFromMaterial(ItemDataRow->MaterialAsset.Get());
    }

    if (TitleText)
    {
        FString CleanNameString = ItemDataRow->Name.ToString();
        CleanNameString = CleanNameString.Replace(TEXT("\""), TEXT(""));
        TitleText->SetText(FText::FromString(CleanNameString));
    }
    CurrentCount = 1;
    MaxCount = NewMaxCount;
    CurrentTradeMode = TradeMode;
    UpdateUI();
}
void UAZQuantitySelectorWidget::UpdateUI()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    UGameInstance* GI = World->GetGameInstance();
    if (GI == nullptr) return;
    UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
    if (DataManger == nullptr) return;

    const FAZStoreDataTableRow* toreDataTableRow = DataManger->GetStoreDataByID(ID);
    if (toreDataTableRow == nullptr) return;
    if (!QuantityInputBox || !PriceText || !ItemSlotImage || !CoinImage) return;

    QuantityInputBox->SetText(FText::AsNumber(CurrentCount));
    FLinearColor NewColor = CurrentCount <= MaxCount ? FLinearColor::White : FLinearColor::Red;
    QuantityInputBox->SetForegroundColor(NewColor);
    ItemSlotImage->SetColorAndOpacity(NewColor);
    PriceText->SetColorAndOpacity(NewColor);
    CoinImage->SetColorAndOpacity(NewColor);

    FText ConfirmTextValue;
    if (CurrentTradeMode == EAZTradeMode::Buy)
    {
        ConfirmTextValue = FText::FromString(TEXT("구매"));
        PriceText->SetText(FText::AsNumber(toreDataTableRow->PurchasePrice * CurrentCount));
    }
    else if (CurrentTradeMode == EAZTradeMode::Sell)
    {
        ConfirmTextValue = FText::FromString(TEXT("판매"));
        PriceText->SetText(FText::AsNumber(toreDataTableRow->BaseValue * CurrentCount));
    }
    ConfirmText->SetText(ConfirmTextValue);
}

void UAZQuantitySelectorWidget::OnPlusClicked()
{
    CurrentCount++;
    UpdateUI();
}


void UAZQuantitySelectorWidget::OnMinusClicked()
{
    if (CurrentCount > 1)
    {
        CurrentCount--;
        UpdateUI();
    }
}

void UAZQuantitySelectorWidget::OnQuantityTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (Text.IsNumeric())
    {
        int32 Val = FCString::Atoi(*Text.ToString());
        CurrentCount = FMath::Max(1, Val);
        UpdateUI();
    }
}


void UAZQuantitySelectorWidget::OnConfirmClicked()
{
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (GI == nullptr) return;
    UAZSoundManagerSubsystem* SoundManger = GI->GetSubsystem<UAZSoundManagerSubsystem>();
    if (SoundManger == nullptr) return;

    if (CurrentCount > MaxCount)
    {
        SoundManger->PlayUISFX(EUISFXType::Click);
        return;
    }
    SoundManger->PlayUISFX(EUISFXType::Transaction);
    OnConfirm.Broadcast(ID, CurrentCount, CurrentTradeMode);
    SetVisibility(ESlateVisibility::Collapsed);
}


void UAZQuantitySelectorWidget::OnCancelClicked()
{
    SetVisibility(ESlateVisibility::Collapsed);
}
