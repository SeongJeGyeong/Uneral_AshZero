// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZTooltipWidget.generated.h"

class UAZItemBase;
class UImage;
class UTextBlock;
class UCanvasPanel;
class UOverlay;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZTooltipWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
public:
	UPROPERTY(EditAnywhere)
	float Width = 500;
	UPROPERTY(EditAnywhere)
	float Height = 400;

	bool UpdateToolTipInfo(UAZItemBase* Item);
	bool UpdateToolTipInfo(int32 TargetID);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> ToolTipOverlay;
protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemTypeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;
private:

};
