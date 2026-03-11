// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Store/AZStoreItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "DataTable/AZStoreDataTableRow.h"
#include "System/AZDataManagerSubsystem.h"
#include "System/Subsystems/AZStoreSubsystem.h"
#include "UI/Store/AZStoreWidget.h"
#include "UI/HUD/AZHUD.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"

void UAZStoreItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    if (Background)
    {
        FLinearColor NewColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

        Background->SetColorAndOpacity(NewColor);
    }

    if (UWorld* World = GetWorld())
    {
        UGameInstance* GI = World->GetGameInstance();
        if (GI == nullptr) return;
        UAZSoundManagerSubsystem* SoundManger = GI->GetSubsystem<UAZSoundManagerSubsystem>();
        if (SoundManger == nullptr) return;

        SoundManger->PlayUISFX(EUISFXType::Hover);   
    }
}

void UAZStoreItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    if (Background)
    {
        FLinearColor NewColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
        Background->SetColorAndOpacity(NewColor);
    }
}

void UAZStoreItemSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!MaterialImage || !GetOwningPlayer())
    {
        return;
    }

    FVector2D MousePosition = FSlateApplication::Get().GetCursorPos();
    bool bCurrentlyUnder = MaterialImage->GetCachedGeometry().IsUnderLocation(MousePosition);

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

FReply UAZStoreItemSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (UWorld* World = GetWorld())
        {
            if (UAZStoreSubsystem* StoreSubsystem = World->GetSubsystem<UAZStoreSubsystem>())
            {
                if (StoreSubsystem->StoreWidget)
                {
                    UGameInstance* GI = World->GetGameInstance();
                    if (GI == nullptr) return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
                    UAZSoundManagerSubsystem* SoundManger = GI->GetSubsystem<UAZSoundManagerSubsystem>();
                    if (SoundManger == nullptr) return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

                    SoundManger->PlayUISFX(EUISFXType::Click);
                    StoreSubsystem->StoreWidget->OnStoreSlotSelected(ID, EAZTradeMode::Buy);
                    return FReply::Handled();
                }
            }
        }
    }

    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UAZStoreItemSlotWidget::InitSlot(FAZStoreDataTableRow* Data)
{
    if (ItemNameText)
    {
        ItemNameText->SetText(FText::FromName(Data->Name));
    }

    if (PriceText)
    {
        PriceText->SetText(FText::AsNumber(Data->PurchasePrice));
    }

    if (MaterialImage)
    {
        if (UGameInstance* GI = GetWorld()->GetGameInstance())
        {
            if (UAZDataManagerSubsystem* DataManager = GI->GetSubsystem<UAZDataManagerSubsystem>())
            {
                UMaterialInstance* MI = DataManager->GetMaterialByID(Data->ID);
                if (MI)
                {
                    MaterialImage->SetBrushFromMaterial(MI);
                }
            }
        }
       
    }

    ID = Data->ID;
}
