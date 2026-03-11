// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UAZCurrencySubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoneyChanged, int32, NewAmount);
/**
 * 
 */
UCLASS()
class ASHZERO_API UUAZCurrencySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	int32 GetMoney() const;
	void AddMoney(int32 Amount);
	bool SpendMoney(int32 Amount);

	UPROPERTY()
	FOnMoneyChanged OnMoneyChanged;
private:
	UPROPERTY()
	int32 CurrentMoney = 1000;
};
