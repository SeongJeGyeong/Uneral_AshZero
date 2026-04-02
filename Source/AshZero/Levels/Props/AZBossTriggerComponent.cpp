// Fill out your copyright notice in the Description page of Project Settings.


#include "Levels/Props/AZBossTriggerComponent.h"
#include "Levels/Rooms/AZBaseRoom.h"
#include "System/GameMode/AZStageGameMode.h"
#include "System/Subsystems/AZSceneSubsystem.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"
#include "System/Subsystems/AZObjectPoolSubsystem.h"
#include "Components/AZHealthComponent.h"
#include "Levels/Props/AZSpawnPointComponent.h"
#include "Enemy/AZBossBase.h"
#include "LevelSequencePlayer.h"
#include "Levels/Props/AZSequencePivot.h"
#include "Character/AZPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AZPlayerMoveComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System/Player/AZPlayerController.h"
#include "Components/SlateWrapperTypes.h"

void UAZBossTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.RemoveDynamic(this, &UAZBossTriggerComponent::OnTriggerBeginOverlap);
	OnComponentBeginOverlap.AddDynamic(this, &UAZBossTriggerComponent::OnTriggerBeginOverlap);

	Owner = Cast<AAZBaseRoom>(GetOwner());
	if (Owner)
	{
		TeleportTransform = Owner->StartPoint->GetComponentTransform();
		TArray<USceneComponent*> ChildrenComp;
		Owner->EnemySpawnPoints->GetChildrenComponents(false, ChildrenComp);
		for (USceneComponent* SpawnPoint : ChildrenComp)
		{
			BossSpawnTransform = SpawnPoint->GetComponentTransform();
		}
	}
}

void UAZBossTriggerComponent::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->IsA(AAZPlayerCharacter::StaticClass())) return;

	if (!Owner->HasAuthority() || bTriggered) return;
	bTriggered = true;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AAZStageGameMode* GameMode = GetWorld()->GetAuthGameMode<AAZStageGameMode>();
	GameMode->TeleportAllPlayersToBossRoom(TeleportTransform);

	PlayBossCutscene_Multicast();
}

void UAZBossTriggerComponent::PlayBossCutscene_Multicast_Implementation()
{
	UAZSceneSubsystem* SceneSystem = GetWorld()->GetGameInstance()->GetSubsystem<UAZSceneSubsystem>();
	if (!SceneSystem) return;

	TArray<AActor*> Actors;
	Owner->GetAttachedActors(Actors);
	for (auto Actor : Actors)
	{
		if (AAZSequencePivot* Pivot = Cast<AAZSequencePivot>(Actor))
		{
			SceneSystem->SequencePivot = Pivot;
			break;
		}
	}

	if (Owner->HasAuthority())
		SceneSystem->OnCutsceneFinished.AddDynamic(this, &UAZBossTriggerComponent::SpawnBoss);

	SceneSystem->PlayBossCutscene(Owner->BossType);

	Owner->OnBossFight.Broadcast(false);
}

void UAZBossTriggerComponent::CutsceneEnd()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->EnableInput(NULL);
		}

		if (AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC))
		{
			AZPC->SetHUDVisibility(ESlateVisibility::HitTestInvisible);
		}

		UAZSoundManagerSubsystem* SoundSystem = GetWorld()->GetGameInstance()->GetSubsystem<UAZSoundManagerSubsystem>();
		if (SoundSystem) SoundSystem->PlayBGM(EBGMType::BGM_Boss);
	}


	SequencePlayer->OnFinished.RemoveDynamic(this, &UAZBossTriggerComponent::CutsceneEnd);
}

void UAZBossTriggerComponent::SpawnBoss()
{
	if (UAZSceneSubsystem* SceneSystem = GetWorld()->GetGameInstance()->GetSubsystem<UAZSceneSubsystem>())
		SceneSystem->OnCutsceneFinished.RemoveDynamic(this, &UAZBossTriggerComponent::SpawnBoss);

	if (bBossSpawned) return;

	UAZObjectPoolSubsystem* ObjectPool = GetWorld()->GetSubsystem<UAZObjectPoolSubsystem>();
	if (!ObjectPool) return;

	AAZBossBase* Boss = ObjectPool->SpawnBoss(Owner->BossType, BossSpawnTransform);
	bBossSpawned = true;

	if (UAZHealthComponent* HealthComp = Boss->FindComponentByClass<UAZHealthComponent>())
		HealthComp->OnDeath.AddDynamic(this, &UAZBossTriggerComponent::OnBossDeath);
}

void UAZBossTriggerComponent::OnBossDeath(FVector DeathLocation, AActor* Killer)
{
	BossDeath_Multicast();
}

void UAZBossTriggerComponent::BossDeath_Multicast_Implementation()
{
	Owner->OnBossFight.Broadcast(true);
	Owner->OnBossFight.Clear();

	UAZSoundManagerSubsystem* SoundSystem = GetWorld()->GetGameInstance()->GetSubsystem<UAZSoundManagerSubsystem>();
	if (SoundSystem) SoundSystem->PlayBGM(EBGMType::BGM_Field);
}
