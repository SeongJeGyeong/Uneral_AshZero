// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "AZBossTriggerComponent.generated.h"

class AAZBaseRoom;
class ULevelSequence;
class ULevelSequencePlayer;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ASHZERO_API UAZBossTriggerComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<ULevelSequence> BossSequence;

	UPROPERTY(Transient)
	TObjectPtr<ULevelSequencePlayer> SequencePlayer;

	UPROPERTY()
	FTransform TeleportTransform;

	UPROPERTY()
	FTransform BossSpawnTransform;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY()
	TObjectPtr<AAZBaseRoom> Owner;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void PlayBossCutscene_Multicast();

	UFUNCTION()
	void CutsceneEnd();

	UFUNCTION()
	void SpawnBoss();

	UFUNCTION()
	void OnBossDeath(FVector DeathLocation, AActor* Killer);

	UFUNCTION(NetMulticast, Reliable)
	void BossDeath_Multicast();

	bool bTriggered = false;
	bool bBossSpawned = false;
};
