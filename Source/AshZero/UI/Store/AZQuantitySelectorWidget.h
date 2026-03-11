// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/AZDefine.h"
#include "AZQuantitySelectorWidget.generated.h"

class UButton;
class UTextBlock;
class USlider;
class UEditableTextBox;
class UImage;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQuantityConfirmed, int32, TargetID, int32, Quantity, EAZTradeMode, Trademode);
UCLASS()
class ASHZERO_API UAZQuantitySelectorWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void InitSelector(int32 TargetID, int32 NewMaxCount, EAZTradeMode TradeMode);

    UPROPERTY(BlueprintAssignable, Category = "AZ|Event")
    FOnQuantityConfirmed OnConfirm;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TitleText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemSlotImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CoinImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PriceText;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ConfirmText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> QuantityInputBox; // 수량 직접 입력

    //UPROPERTY(meta = (BindWidget))
    //TObjectPtr<USlider> QuantitySlider;      // 수량 슬라이더

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> PlusButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> MinusButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ConfirmButton;

    //UPROPERTY(meta = (BindWidget))
    //TObjectPtr<UTextBlock> ConfirmButtonText; // "구매" / "판매"

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> CancelButton;
private:
    int32 ID = 0;
    int32 MaxCount = 0;
    bool bIsHovering = false;

    UFUNCTION()
    void OnPlusClicked();

    UFUNCTION()
    void OnMinusClicked();

    UFUNCTION()
    void OnQuantityTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

    UFUNCTION()
    void OnConfirmClicked();

    UFUNCTION()
    void OnCancelClicked();

    void UpdateUI();

    int32 CurrentCount = 1;

    EAZTradeMode CurrentTradeMode = EAZTradeMode::Buy;
};
