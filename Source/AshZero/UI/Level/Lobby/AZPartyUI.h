// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZPartyUI.generated.h"

class UWidgetSwitcher;
class UTextBlock;
class UVerticalBox;
class UAZQuestObjListItem;
class UProgressBar;
class UButton;
class UCanvasPanel;
class UAZPartyIcon;
class UOverlay;
class UImage;
class UTextBlock;

UCLASS()
class ASHZERO_API UAZPartyUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> QuestUI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> QuestName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UVerticalBox> ObjectiveList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UProgressBar> QuestBar;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAZQuestObjListItem> ObjectiveItemClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> RoadOutBtn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UAZPartyIcon> Player1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UAZPartyIcon> Player2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> DestroySessionBtn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> ExitSessionBtn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> GameStartBtn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> ReadyBtn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldAmount;
	
	void SetVisibilityPartyUI(bool bIsVisible);

	// 캐릭터 프리뷰
	// Local
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UOverlay> LocalOverlay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> LocalImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> LocalName;

	// Other
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UOverlay> OtherOverlay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> OtherImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> OtherName;

	UFUNCTION()
	void UpdatePartyUI();
private:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedRoadOut();

	UFUNCTION()
	void OnClickedReady();

	UFUNCTION()
	void OnCurrencyChanged(int32 NewAmount);
};
