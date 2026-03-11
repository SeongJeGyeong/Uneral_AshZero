// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZInteractionWidget.generated.h"

class UTextBlock;

UCLASS()
class ASHZERO_API UAZInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	void UpdatePositionInViewport(FMargin ScreenMargin, float ScreenRadiusPercent);
	void SetInteractionText(FKey Key, FText Info);

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	UPROPERTY()
	FVector LastWorldLocation;

private:
	void SetInteractionPercent(float Percent);

	bool IsOnScreen(FMargin Margin);

	
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UTextBlock> InfoText;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "AZ|UI")
	TObjectPtr<UTextBlock> InteractionKey;
};
