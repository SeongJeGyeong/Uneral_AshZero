// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AZQuestObjectiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/AZEnemyBase.h"
#include "Components/AZHealthComponent.h"
#include "Interactables/AZNonPlayerCharacter.h"
#include "System/Subsystems/AZQuestManagerSubsystem.h"

UAZQuestObjectiveComponent::UAZQuestObjectiveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UAZQuestObjectiveComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = GetOwner();
	if (!Owner) return;

	if (Owner->IsA(AAZEnemyBase::StaticClass()))
	{
		UAZHealthComponent* HealthComp = Owner->FindComponentByClass<UAZHealthComponent>();
		if (HealthComp)
		{
			HealthComp->OnDeath.AddDynamic(this, &UAZQuestObjectiveComponent::OnDeathOwner);
		}
	}
	else if (Owner->IsA(AAZNonPlayerCharacter::StaticClass()))
	{
		Cast<AAZNonPlayerCharacter>(Owner)->OnInteraction.AddDynamic(this, &UAZQuestObjectiveComponent::OnInteractNPC);
	}
}

void UAZQuestObjectiveComponent::OnDeathOwner(FVector DeathLocation, AActor* Killer)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(Killer, 0);
	if (!PC || !PC->IsLocalController()) return;

	SendObjectiveProgress(Killer, ObjectiveTag, EQuestObjectiveType::Slay, 1);
}

void UAZQuestObjectiveComponent::OnInteractNPC(AActor* NPC)
{
	SendObjectiveProgress(NPC, ObjectiveTag, EQuestObjectiveType::Talk, 1);
}

void UAZQuestObjectiveComponent::SendObjectiveProgress(const UObject* WorldContextObject, const FGameplayTag& Tag, EQuestObjectiveType Type, int32 Count)
{
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return;
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance) return;
	UAZQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UAZQuestManagerSubsystem>();
	if (!QuestManager) return;
	QuestManager->UpdateQuestProgress(Tag, Type, Count);
}