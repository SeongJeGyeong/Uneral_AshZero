// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "AZSessionItem.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;

UCLASS()
class ASHZERO_API UAZSessionItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void UpdateSessionInfo(const FBlueprintSessionResult& SearchResult);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FBlueprintSessionResult SessionResult;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true, BindWidget))
	TObjectPtr<UButton> SessionItem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true, BindWidget))
	TObjectPtr<UTextBlock> SessionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true, BindWidget))
	TObjectPtr<UTextBlock> NumOfPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true, BindWidget))
	TObjectPtr<UTextBlock> Ping;

};
