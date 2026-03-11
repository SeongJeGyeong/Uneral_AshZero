// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZCutsceneWidget.generated.h"

class UImage;
class UTextBlock;
class UWidgetAnimation;

UCLASS()
class ASHZERO_API UAZCutsceneWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UImage> SceneImage;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> SceneText;

	//UPROPERTY(Transient, meta = (BindWidgetAnim))
	//TObjectPtr<UWidgetAnimation> TextFadeAnim;

	//void PlayTextAnim(float CutsceneDuration);

	//void SetCutsceneDuration(float Duration) const { CutsceneDuration = Duration; }

//private:
//	float CutsceneDuration = 7.f;
};
