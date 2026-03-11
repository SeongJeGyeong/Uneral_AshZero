// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZItemImagetWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZItemImagetWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
public:
	void SetUp(int32 NewID, UMaterialInstance* MI);
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> ItemImage;
private:
	int32 ID = 0;
};
