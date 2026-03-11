// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Subsystems/UAZCurrencySubsystem.h"

void UUAZCurrencySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	//TODO: 세이브 한 돈을 여기서 복구
}

int32 UUAZCurrencySubsystem::GetMoney() const
{
	return CurrentMoney;
}

void UUAZCurrencySubsystem::AddMoney(int32 Amount)
{
	CurrentMoney += Amount;
	OnMoneyChanged.Broadcast(CurrentMoney);
}

bool UUAZCurrencySubsystem::SpendMoney(int32 Amount)
{
	if (CurrentMoney >= Amount)
	{
		CurrentMoney -= Amount;
		OnMoneyChanged.Broadcast(CurrentMoney);
		return true;
	}
	return false;
}
