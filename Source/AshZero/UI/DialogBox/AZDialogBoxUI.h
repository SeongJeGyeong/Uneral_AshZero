// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/AZDefine.h"
#include "Misc/Optional.h"
#include "AZDialogBoxUI.generated.h"

class UVerticalBox;
class UAZDialogButtonWidget;
class UTextBlock;
class URichTextBlock;
class UButton;
class UAZDialogSubsystem;
class UAZQuestManagerSubsystem;
class AAZPlayerController;

UCLASS()
class ASHZERO_API UAZDialogBoxUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void SetActorDialog(FGameplayTag GiverTag, FEnableButtons EnableButtons);

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> DialogTable;

	TOptional<FAZDialog> CurrentDialog;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ButtonWrapper;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerName;	

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> DialogText;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAZDialogButtonWidget> ButtonClass;

	int32 CurrentLogIndex = 0;

	UPROPERTY()
	TArray<FAZDialog> TalkDialog;

	UPROPERTY()
	FAZDialog CraftingDialog;

	UPROPERTY()
	FAZDialog EnhanceDialog;

	FGameplayTag NPCTag;
	FText NameText;

	void CreateFeatureButton(EDialogButtonType ButtonType, FText ButtonText);

	using FButtonHandlerFunc = void (UAZDialogBoxUI::*)(UButton*);
	void CreateFeatureButton(FText ButtonText, FButtonHandlerFunc ButtonHandler);

	UFUNCTION()
	void OnConversationButtonClicked(UButton* ClickedButton);

	UFUNCTION()
	void OnCraftingButtonClicked(UButton* ClickedButton);

	UFUNCTION()
	void OnQuestButtonClicked(UButton* ClickedButton);

	UFUNCTION()
	void OnQuestAcceptButtonClicked(UButton* ClickedButton);

	UFUNCTION()
	void OnQuestDeclineButtonClicked(UButton* ClickedButton);

	UFUNCTION()
	void OnTradeButtonClicked(UButton* ClickedButton);

	UFUNCTION()
	void OnEnhanceButtonClicked(UButton* ClickedButton);

	UFUNCTION()
	void OnSessionButtonClicked(UButton* ClickedButton);

	TWeakObjectPtr<UAZDialogSubsystem> DialogSubsystem;
	TWeakObjectPtr<UAZQuestManagerSubsystem> QuestSubsystem;
	TWeakObjectPtr<AAZPlayerController> OwnerPC;

	void EndDialogConversation();

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void GoToNextLine();

	void VisibilityChanged(ESlateVisibility InVisibility);

	void ExitDialogUI();
	void SetButtonVisibility(bool bVisible);
};
