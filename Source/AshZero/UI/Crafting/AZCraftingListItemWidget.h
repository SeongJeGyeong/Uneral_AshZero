// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/AZDefine.h"
#include "AZCraftingListItemWidget.generated.h"


class UImage;
class USizeBox;
class UTextBlock;
class UProgressBar;
class UAZCraftingComponent;
class UAZItemImagetWidget;
class UOverlay;
struct FAZCraftingRecipeDataTableRow;
/**
 * 
 */
class UWidgetAnimation;

UCLASS()
class ASHZERO_API UAZCraftingListItemWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
public:
	//필요 재화 UI
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<USizeBox> CostSizeBox;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> CostText;

	//첫 번째 필요 재료 UI
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UAZItemImagetWidget> MaterialImage_1;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> Material_1_Qty;

	//두 번째 필요 재료 UI
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UAZItemImagetWidget> MaterialImage_2;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> Material_2_Qty;

	//네 번째 필요 재료 UI
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UAZItemImagetWidget> MaterialImage_3;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> Material_3_Qty;

	// 아이템 슬롯과 골드 UI
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> GoldImage;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> SlotImage_1;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> SlotImage_2;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> SlotImage_3;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> SlotImage_4;

	//결과 아이템 UI
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UAZItemImagetWidget> ResultItemImage;
	
	//남은 시간 UI
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> RemainingTimeText; 
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<USizeBox> RemainingSizeBox;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UProgressBar> CraftingProgressBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UOverlay> GlowOverlay;

	void Setup(UAZCraftingComponent* InComp, FAZCraftingRecipeDataTableRow* InCraftingRecipe, const FItemAmountMap& AmountMap);
	
	UFUNCTION()
	void HandleCraftingReadyToCollect(int32 InJobID);
	UFUNCTION()
	void HandleCraftingFinished(int32 CompletedJobID);
	
	UFUNCTION()
	void UpdateAvailability(const FItemAmountMap& AmountMap);

	//위젯 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> CraftingFinishAnimation;
private:
	int32 JobId;
	FAZCraftingRecipeDataTableRow* CachedRecipe;
	TWeakObjectPtr<UAZCraftingComponent> CraftingComp;
};
