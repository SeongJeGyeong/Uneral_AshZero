// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZExitUI.generated.h"

class UImage;
class UWidgetSwitcher;
class UMaterialInstanceDynamic;

UCLASS()
class ASHZERO_API UAZExitUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> ProgressGuage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressBarMID;

	float Percent = 0.f;

	bool bPlaying = false;

	int32 CachedIndex = 0;
	static constexpr int32 MaxStepCount = 5;

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void SetPercent(float Figure);
};
