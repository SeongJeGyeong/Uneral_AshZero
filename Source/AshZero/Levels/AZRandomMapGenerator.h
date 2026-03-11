// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Util/AZDefine.h"
#include "AZRandomMapGenerator.generated.h"

class AAZBaseRoom;
class AAZChest;
class ANavMeshBoundsVolume;
class ULevelStreamingDynamic;
class AAZDoor;
class UAZRoomDataAsset;
class UNiagaraSystem;
class UNiagaraComponent;
class UAZRoomExitNode;

USTRUCT(BlueprintType)
struct FRoomData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> LevelAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoundsExtent = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoundsOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTransform> LocalExitTransforms;
};

USTRUCT(BlueprintType)
struct FSpawnLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ShowOnlyInnerProperties))
	FRoomData RoomData;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 Amount = 0;
};

USTRUCT()
struct FRoomSpawnContext
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UWorld> LevelAsset;

	UPROPERTY()
	FTransform SpawnTransform = FTransform::Identity;

	UPROPERTY()
	AAZBaseRoom* ParentRoom = nullptr;

	UPROPERTY()
	ULevelStreamingDynamic* StreamingHandle = nullptr;

	UPROPERTY()
	TObjectPtr<AAZDoor> EntranceDoor = nullptr;

	UPROPERTY()
	int32 RoomIdx = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<UAZRoomExitNode> ParentNode = nullptr;

public:
	void ResetContext()
	{
		LevelAsset = nullptr;
		SpawnTransform = FTransform::Identity;
		ParentRoom = nullptr;
		StreamingHandle = nullptr;
	}

	bool ValidCheck() const
	{
		const bool bIsValid = !LevelAsset.IsNull() && StreamingHandle != nullptr;
		if(!bIsValid)
		{
			if (LevelAsset.IsNull()) UE_LOG(LogTemp, Error, TEXT("[RoomSpawnContext] LevelAsset is null"));
			if (!StreamingHandle)    UE_LOG(LogTemp, Error, TEXT("[RoomSpawnContext] StreamingHandle is null"));
		}

		return bIsValid;
	}
};

USTRUCT(BlueprintType)
struct FSimulationData
{
	GENERATED_BODY()

	UPROPERTY()
	FRoomData RoomData;

	UPROPERTY()
	int32 RoomIndex = INDEX_NONE;

	UPROPERTY()
	FTransform WorldTransform = FTransform::Identity;

	UPROPERTY()
	int32 ParentRoomIndex = INDEX_NONE;

	UPROPERTY()
	int32 ParentExitIndex = INDEX_NONE;

	UPROPERTY()
	TArray<TObjectPtr<UAZRoomExitNode>> ExitNodes;

	UPROPERTY()
	TObjectPtr<UAZRoomExitNode> ParentExitNode = nullptr;

	UPROPERTY()
	TObjectPtr<AAZBaseRoom> SpawnedRoom = nullptr;

	UPROPERTY()
	int32 DistanceFromStart = -1;  // 시작 룸으로부터의 거리

	UPROPERTY()
	bool bIsBossRoom = false;      // 보스 룸 여부

	UPROPERTY()
	bool bIsEscapeRoom = false;    // 탈출 지점 여부
};

UCLASS()
class ASHZERO_API AAZRandomMapGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	AAZRandomMapGenerator();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data For Generate")
	TObjectPtr<UAZRoomDataAsset> RoomDataAsset;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Data For Generate")
	TArray<EBossType> BossList;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Data For Generate")
	TSubclassOf<AActor> DoorClass;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Data For Generate")
	TSubclassOf<AAZChest> TreasureClass;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Data For Generate")
	TSubclassOf<AActor> TeleporterClass;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Data For Generate")
	int32 TreasureAmount = 0;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Data For Generate")
	int32 TeleportPointAmount = 0;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Data For Generate")
	int32 Seed = -1;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Simulation Option")
	bool bIsSimulate = false;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Simulation Option")
	TSubclassOf<APawn> SpectorClass;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Simulation Option")
	TObjectPtr<UNiagaraSystem> SimulationCube;

	UPROPERTY(ReplicatedUsing = OnRep_Seed)
	int32 NetSeed = -1;

	UPROPERTY()
	TArray<FSimulationData> RoomSpawnInfos;

public:
	void SetSeedAndGenerate(int32 NewSeed);
	void SpawnReserveBound(const FRoomData& RoomData, const FTransform& SpawnTransform);
	void CollectRoomInfo(AAZBaseRoom* SpawnedRoom);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	template<typename T>
	bool GetRandomArrayItemFromStream(const TArray<T>& InArray, FRandomStream& Stream, T& OutItem, int32& OutIndex)
	{
		OutIndex = INDEX_NONE;
		if (InArray.IsEmpty()) return false;

		OutIndex = Stream.RandRange(0, InArray.Num() - 1);
		OutItem = InArray[OutIndex];
		return true;
	}

protected:
	virtual void BeginPlay() override;

private:
	void GenerateMap(int32 SeedValue);
	void SimulateMapGenerate();
	void CreateSimulationData(const FRoomData& RoomData, UAZRoomExitNode* ParentNode, const FTransform& Entrance);
	void CreateRealMap(int32 ParentID = -1);
	void ResetGenerationState();

	void RoomSpawn(FSimulationData& SpawnData, UAZRoomExitNode* EntranceNode);
	bool TryPlaceRoom(const FRoomData& RoomData, UAZRoomExitNode* ParentNode, const FTransform& ExitTransform);
	void FinalizeSpecialRooms(TArray<UAZRoomExitNode*>& ExitNodeList, int32 PlacedBossCount, int32 TargetBossCount, int32 PlacedEscapeCount, int32 TargetEscapeCount);
	bool CanPlaceBossRoom(UAZRoomExitNode* ParentExitNode);
	bool CanPlaceEscapeRoom(UAZRoomExitNode* ParentExitNode);

	int32 CalculateRoomDistance(int32 FromRoomIndex, int32 ToRoomIndex) const;

	void CloseDoors();
	void LinkDoors(AAZBaseRoom* Room, const FRoomSpawnContext& Context);
	void LinkAdjacentRoom(AAZBaseRoom* Room, const FRoomSpawnContext& Context);
	void GenerateContents();

	void SpawnTreasureBox();
	void SpawnTeleporter();
	void UpdateNavMeshBounds();
	void CreateSimulationBox(const FVector& Location, const FRotator& Rotation, const FVector& Extent);

	UFUNCTION()
	void OnRep_Seed();

	UFUNCTION()
	void OnRoomLevelSpawned();

	AAZBaseRoom* FindSpawnedRoomInLevel(ULevel* Level) const;
	FString GetNetModeString() const;

private:
	UPROPERTY(Transient)
	TArray<FSpawnLevelData> RoomLevelAssets;

	UPROPERTY(Transient)
	TArray<FTransform> TreasureList;

	UPROPERTY(Transient)
	TArray<FTransform> TeleporterList;

	UPROPERTY(Transient)
	FRandomStream RandomStream;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> ReserveList;

	UPROPERTY(Transient)
	TObjectPtr<AAZBaseRoom> StartRoom;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AAZDoor>> DoorList;

	// 현재 로딩 중인 레벨을 추적하기 위한 포인터
	UPROPERTY(Transient)
	TObjectPtr <class ULevelStreamingDynamic> CurrentLoadingLevel;

	UPROPERTY(Transient)
	FRoomSpawnContext CurrentSpawnLevel;

	UPROPERTY(Transient)
	TArray<int32> BossRoomIndices;

	UPROPERTY(Transient)
	TArray<int32> EscapeRoomIndices;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UNiagaraComponent>> NiagaraList;

	UPROPERTY()
	FTransform StartPoint = FTransform::Identity;

	float MinX = TNumericLimits<float>::Max();
	float MaxX = TNumericLimits<float>::Lowest();
	float MinY = TNumericLimits<float>::Max();
	float MaxY = TNumericLimits<float>::Lowest();

	int32 SpawnIdx = 0;
	int32 NodeIdIdx = 0;
	bool bSpawnedStartLevel = false;
};