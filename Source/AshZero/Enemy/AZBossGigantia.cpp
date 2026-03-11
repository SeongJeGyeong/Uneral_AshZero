// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/AZBossGigantia.h"
#include "Enemy/AZBossAIController.h"
#include "GameplayTagsManager.h"
#include "AZGameplayTags.h"
#include "AshZero.h"

AAZBossGigantia::AAZBossGigantia()
{
}

void AAZBossGigantia::BeginPlay()
{
	Super::BeginPlay();

	PRINT_LOG(TEXT("Gigantia Boss Initialized - Phase: %d"), CurrentPhase);
}
