// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_CheckBossBehavior.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTDecorator_CheckBossBehavior::UBTDecorator_CheckBossBehavior()
{
	NodeName = TEXT("Check Boss Behavior");

	BehaviorKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckBossBehavior, BehaviorKey), StaticEnum<EBossBehavior>());
}

bool UBTDecorator_CheckBossBehavior::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return false;
	}

	EBossBehavior CurrentBehavior = static_cast<EBossBehavior>(BB->GetValueAsEnum(BehaviorKey.SelectedKeyName));
	return CurrentBehavior == ExpectedBehavior;
}

FString UBTDecorator_CheckBossBehavior::GetStaticDescription() const
{
	FString BehaviorName;
	switch (ExpectedBehavior)
	{
	case EBossBehavior::Idle:
		BehaviorName = TEXT("Idle");
		break;
	case EBossBehavior::Turn:
		BehaviorName = TEXT("Turn");
		break;
	case EBossBehavior::Strafe:
		BehaviorName = TEXT("Strafe");
		break;
	case EBossBehavior::Attack:
		BehaviorName = TEXT("Attack");
		break;
	default:
		BehaviorName = TEXT("Unknown");
		break;
	}

	return FString::Printf(TEXT("Behavior == %s"), *BehaviorName);
}