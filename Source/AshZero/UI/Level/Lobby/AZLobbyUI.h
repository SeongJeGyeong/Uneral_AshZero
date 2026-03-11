// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AZLobbyUI.generated.h"

class UButton;
class UScrollBox;

UCLASS()
class ASHZERO_API UAZLobbyUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> CreateSessionBtn;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> FindSessionBtn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UScrollBox> SessionList;

public:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnClickCreateSessionBtn();
	UFUNCTION()
	void OnClickFindSessionBtn();

	FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	void VisibilityChanged(ESlateVisibility InVisibility);
};