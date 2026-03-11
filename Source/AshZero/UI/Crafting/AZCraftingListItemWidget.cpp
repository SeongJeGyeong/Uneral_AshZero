// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Crafting/AZCraftingListItemWidget.h"
#include "Components/AZCraftingComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"
#include "DataTable/AZCraftingRecipeDataTableRow.h"
#include "System/AZDataManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "System/Subsystems/UAZCurrencySubsystem.h"
#include "UI/Inventory/AZItemImagetWidget.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"

void UAZCraftingListItemWidget::Setup(UAZCraftingComponent* InComp, FAZCraftingRecipeDataTableRow* InCraftingRecipe, const FItemAmountMap& AmountMap)
{
	if (InComp == nullptr)
		return;
	CraftingComp = InComp;
	CachedRecipe = InCraftingRecipe;
	JobId = InCraftingRecipe->JobID;
	CraftingComp->OnCraftingFinished.RemoveDynamic(this, &UAZCraftingListItemWidget::HandleCraftingFinished);
	CraftingComp->OnCraftingFinished.AddDynamic(this, &UAZCraftingListItemWidget::HandleCraftingFinished);
	
	CraftingComp->OnCraftingReadyToCollect.RemoveDynamic(this, &UAZCraftingListItemWidget::HandleCraftingReadyToCollect);
	CraftingComp->OnCraftingReadyToCollect.AddDynamic(this, &UAZCraftingListItemWidget::HandleCraftingReadyToCollect);

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;

	UMaterialInstance* Material_1 = DataManger->GetMaterialByID(InCraftingRecipe->Mat_1_ID);
	UMaterialInstance* Material_2 = DataManger->GetMaterialByID(InCraftingRecipe->Mat_2_ID);
	UMaterialInstance* Material_3 = DataManger->GetMaterialByID(InCraftingRecipe->Mat_3_ID);
	UMaterialInstance* ResultMaterial = DataManger->GetMaterialByID(InCraftingRecipe->ResultItemID);
	if (Material_1 == nullptr || Material_2 == nullptr || Material_3 == nullptr || ResultMaterial == nullptr)
		return;

	MaterialImage_1->SetUp(InCraftingRecipe->Mat_1_ID, Material_1);
	MaterialImage_2->SetUp(InCraftingRecipe->Mat_2_ID, Material_2);
	MaterialImage_3->SetUp(InCraftingRecipe->Mat_3_ID, Material_3);
	ResultItemImage->SetUp(InCraftingRecipe->ResultItemID, ResultMaterial);

	Material_1_Qty->SetText(FText::AsNumber(InCraftingRecipe->Mat_1_Qty));
	Material_2_Qty->SetText(FText::AsNumber(InCraftingRecipe->Mat_2_Qty));
	Material_3_Qty->SetText(FText::AsNumber(InCraftingRecipe->Mat_3_Qty));
	CostText->SetText(FText::AsNumber(InCraftingRecipe->Cost));
	UpdateAvailability(AmountMap);

	if (GlowOverlay) GlowOverlay->SetVisibility(ESlateVisibility::Collapsed);
}

void UAZCraftingListItemWidget::HandleCraftingReadyToCollect(int32 InJobID)
{
	if (JobId == InJobID)
	{
		if (CraftingProgressBar)
		{
			CraftingProgressBar->SetPercent(0.0f);
		}

		if (CostSizeBox) CostSizeBox->SetVisibility(ESlateVisibility::Collapsed);
		if (RemainingSizeBox) RemainingSizeBox->SetVisibility(ESlateVisibility::Collapsed);
		if (GlowOverlay) GlowOverlay->SetVisibility(ESlateVisibility::Visible);
		if (CraftingFinishAnimation)
		{
			UGameInstance* GI = GetWorld()->GetGameInstance();
			if (GI == nullptr) return;
			UAZSoundManagerSubsystem* SoundManger = GI->GetSubsystem<UAZSoundManagerSubsystem>();
			if (SoundManger == nullptr) return;
			SoundManger->PlayUISFX(EUISFXType::Craft);
			PlayAnimation(CraftingFinishAnimation);
		}
	}

}

void UAZCraftingListItemWidget::HandleCraftingFinished(int32 CompletedJobID)
{
	if (JobId == CompletedJobID)
	{
		if (RemainingSizeBox)
		{
			RemainingSizeBox->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (CraftingProgressBar)
		{
			CraftingProgressBar->SetPercent(0);
			CostSizeBox->SetVisibility(ESlateVisibility::Visible);
		}
		if (GlowOverlay) GlowOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAZCraftingListItemWidget::UpdateAvailability(const FItemAmountMap& AmountMap)
{
	if (CachedRecipe == nullptr || GetWorld() == nullptr)
		return;
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UUAZCurrencySubsystem* CurrencySystem = GI->GetSubsystem<UUAZCurrencySubsystem>();
	if (CurrencySystem == nullptr)
		return;
	bool bEnoughMoney = CurrencySystem->GetMoney() >= CachedRecipe->Cost;
	FLinearColor MoneyColor = bEnoughMoney ? FLinearColor::White : FLinearColor::Red;
	if (GoldImage && CostText)
	{
		GoldImage->SetColorAndOpacity(MoneyColor);
		CostText->SetColorAndOpacity(MoneyColor);
	}

	auto CheckAndSetColor = [&](int32 ItemID, int32 RequiredQty, UImage* SlotImg, UTextBlock* QtyText) {
		if (ItemID < 0) return;

		int32 CurrentOwnedQty = 0;
		if (const int32* FoundQty = AmountMap.Map.Find(ItemID))
			CurrentOwnedQty = *FoundQty;

		bool bEnoughMaterial = (CurrentOwnedQty >= RequiredQty);
		FLinearColor SlotColor = bEnoughMaterial ? FLinearColor::White : FLinearColor::Red;
		if (SlotImg && QtyText)
		{
			SlotImg->SetColorAndOpacity(SlotColor);
			QtyText->SetColorAndOpacity(FSlateColor(SlotColor));
		}
	};

	CheckAndSetColor(CachedRecipe->Mat_1_ID, CachedRecipe->Mat_1_Qty, SlotImage_1, Material_1_Qty);
	CheckAndSetColor(CachedRecipe->Mat_2_ID, CachedRecipe->Mat_2_Qty, SlotImage_2, Material_2_Qty);
	CheckAndSetColor(CachedRecipe->Mat_3_ID, CachedRecipe->Mat_3_Qty, SlotImage_3, Material_3_Qty);
}

FReply UAZCraftingListItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (CraftingComp.IsValid())
	{
		UGameInstance* GI = GetWorld()->GetGameInstance();
		if (GI == nullptr) return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
		UAZSoundManagerSubsystem* SoundManger = GI->GetSubsystem<UAZSoundManagerSubsystem>();
		if (SoundManger == nullptr) return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

		if (CraftingComp->IsCraftingReadyToCollect(JobId))
		{
			CraftingComp->CollectCraftedItem(JobId);
		}
		else if (!CraftingComp->IsCraftingItem(JobId))
		{
			if (CraftingComp->TryStartCrafting(JobId))
			{
				CostSizeBox->SetVisibility(ESlateVisibility::Collapsed);
				RemainingSizeBox->SetVisibility(ESlateVisibility::Visible);
			}
		}

		SoundManger->PlayUISFX(EUISFXType::Click);
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

}

void UAZCraftingListItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (BackgroundImage)
	{
		BackgroundImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
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

void UAZCraftingListItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (BackgroundImage)
	{
		BackgroundImage->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 0.7f));
	}
}

void UAZCraftingListItemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (CraftingComp != nullptr)
	{
		if (CraftingComp->IsCraftingItem(JobId))
		{
			float Progress = CraftingComp->GetCraftingProgress(JobId);
			float RemainTime = CraftingComp->GetRemainingTime(JobId);

			CraftingProgressBar->SetPercent(Progress);
			RemainingTimeText->SetText(FText::AsNumber((int32)RemainTime));
		}
	}
}
