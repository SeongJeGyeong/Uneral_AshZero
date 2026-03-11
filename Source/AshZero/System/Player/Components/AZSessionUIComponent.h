// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AZSessionUIComponent.generated.h"

class UAZLobbyUI;
class UAZPartyUI;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZSessionUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZSessionUIComponent();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAZLobbyUI> SessionUIClass;
	UPROPERTY()
	TObjectPtr<UAZLobbyUI> SessionUI;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAZPartyUI> PartyUIClass;
	UPROPERTY()
	TObjectPtr<UAZPartyUI> PartyUI;

public:
	void ConstructUI();
	void SetSessionUI(ESlateVisibility Visibility);

	UFUNCTION(BlueprintCallable)
	void OpenSessionUI();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<class AAZPlayerController> Owner;

	bool bIsVisible = true;
};
