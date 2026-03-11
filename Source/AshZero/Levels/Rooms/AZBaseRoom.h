// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Util/AZDefine.h"
#include "AZBaseRoom.generated.h"

class UArrowComponent;
class UBoxComponent;
class UAZSpawnPointComponent;
class ULevelStreamingDynamic;
class AAZEnemyBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossFight, bool, bDoorOpen);

UCLASS()
class ASHZERO_API AAZBaseRoom : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AAZBaseRoom();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> OverlapTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Props;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> ExitPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> ItemSpawnPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> EnemySpawnPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> TeleportPoint;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UArrowComponent> RoomDirection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAZSpawnPointComponent> StartPoint;

	UPROPERTY()
	int32 CountFromStart = 0;

	UPROPERTY()
	TArray<TObjectPtr<AAZBaseRoom>> AdjacentRooms;

	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> StreamingHandle = nullptr;

	int32 PlayerCount = 0;
	bool bEnemiesSpawned = false;
	ERoomType RoomType = ERoomType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBossType BossType = EBossType::SpineQueen;

	FOnBossFight OnBossFight;

	UPROPERTY()
	TObjectPtr<UWorld> PersistentWorld = nullptr;

public:
	void ActivateRoom(bool bActive);
	void EmptySpawnPoint();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnRoomLevelShown();

private:
	void DormantContainedEnemies();
	void WakeDormantEnemies();
	bool IsEnemyFullyInsideOverlapTrigger(const AAZEnemyBase* Enemy) const;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AAZEnemyBase>> DormantEnemies;
};
