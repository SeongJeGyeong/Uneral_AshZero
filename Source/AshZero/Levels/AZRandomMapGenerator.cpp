// Fill out your copyright notice in the Description page of Project Settings.


#include "Levels/AZRandomMapGenerator.h"
#include "Levels/Rooms/AZBaseRoom.h"
#include "Components/BoxComponent.h"
#include "Interactables/AZChest.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "System/GameMode/AZStageGameMode.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "Components/BrushComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Levels/Props/AZDoor.h"
#include "System/Player/AZPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "DataAsset/AZRoomDataAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"
#include "System/AZSessionSubsystem.h"
#include "Levels/AZRoomExitNode.h"
#include "Levels/Props/AZSpawnPointComponent.h"

AAZRandomMapGenerator::AAZRandomMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AAZRandomMapGenerator::BeginPlay()
{
	Super::BeginPlay();	

	if (!HasAuthority()) return;

	if (AAZPlayerController* PC = Cast<AAZPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
		PC->StartMapGenerate();
}

void AAZRandomMapGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAZRandomMapGenerator, NetSeed);
}

void AAZRandomMapGenerator::SetSeedAndGenerate(int32 NewSeed)
{
	if (!HasAuthority()) return;

	NetSeed = NewSeed;
	GenerateMap(NetSeed);
}

void AAZRandomMapGenerator::OnRep_Seed()
{
	GenerateMap(NetSeed);
}

void AAZRandomMapGenerator::GenerateMap(int32 SeedValue)
{
	const TArray<EBossType> TempBossList = BossList;
	const int32 BaseSeed = (bIsSimulate || TestSeed > 0) ? TestSeed : SeedValue;

	if (bIsSimulate)
	{
		const FVector SpawnLocation = FVector(0.f, 0.f, 25000.f);
		if (APawn* Spector = GetWorld()->SpawnActor<APawn>(SpectatorClass, SpawnLocation, FRotator::ZeroRotator))
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
				PC->Possess(Spector);
		}
	}
	for (int32 Retry = 0; Retry < MaxRetryCount; ++Retry)
	{
		ResetGenerationState();

		DeterminedSeed = BaseSeed + Retry;
		UE_LOG(LogTemp, Warning, TEXT("[GenerateMap] Seed: %d"), DeterminedSeed);
		RandomStream.Initialize(DeterminedSeed);
		BossList = TempBossList;

		RoomLevelAssets = RoomDataAsset->RoomDataList;

		const bool bSuccess = SimulateRoomPlacement();

		if (bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GenerateMap] 맵 생성 성공. 시도 횟수: %d"), Retry + 1);
			break;
		}

		UE_LOG(LogTemp, Warning, TEXT("[GenerateMap] %d회 실패, %s"), Retry + 1,
			(Retry + 1 < MaxRetryCount) ? TEXT("재시작") : TEXT("마지막 결과 배치"));
	}

	FindRoomInfoById(INDEX_NONE);
}

bool AAZRandomMapGenerator::SimulateRoomPlacement()
{
	// 시작 룸 후보 필터(출구 2개 이상)
	TArray<int32> StartCandidates;
	StartCandidates.Reserve(RoomLevelAssets.Num());

	int32 RoomAmount = 0;
	for (const FSpawnLevelData& Level : RoomLevelAssets)
	{
		RoomAmount += Level.Amount;
		if (Level.RoomData.LocalExitTransforms.Num() > 1)
			StartCandidates.Add(&Level - RoomLevelAssets.GetData());
	}

	if (StartCandidates.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("시작 룸의 출구는 2개 이상이어야 함"));
		return false;
	}

	// 시작 룸 결정
	const int32 index = StartCandidates[RandomStream.RandRange(0, StartCandidates.Num() - 1)];
	FSpawnLevelData StartRoomData = RoomLevelAssets[index];

	const int32 TargetTotalRooms = RoomAmount;

	if (--RoomLevelAssets[index].Amount <= 0)
		RoomLevelAssets.RemoveAtSwap(index);

	--RoomAmount;

	// 시작 룸 AABB 등록 (TryPlaceRoom을 거치지 않으므로 직접 추가)
	const FBox StartLocalBox(-StartRoomData.RoomData.BoundsExtent, StartRoomData.RoomData.BoundsExtent);
	const FTransform StartBoxTransform(FQuat::Identity, StartRoomData.RoomData.BoundsOffset);
	PlacedBounds.Add(StartLocalBox.TransformBy(StartBoxTransform));

	FSimulationData StartData;
	StartData.RoomData = StartRoomData.RoomData;
	StartData.RoomIndex = 0;
	StartData.ParentExitNode = nullptr;
	StartData.ParentRoomIndex = INDEX_NONE;
	StartData.DistanceFromStart = 0;

	for (const FTransform& LocalExit : StartRoomData.RoomData.LocalExitTransforms)
	{
		UAZRoomExitNode* ExitNode = NewObject<UAZRoomExitNode>(this);
		ExitNode->NodeID = NodeIdIdx++;
		ExitNode->OwnerIndex = StartData.RoomIndex;
		ExitNode->ExitTransform = LocalExit;
		StartData.ExitNodes.Add(ExitNode);
	}

	RoomSpawnInfos.Add(StartData);

	TArray<UAZRoomExitNode*> ExitNodeList;
	ExitNodeList.Reserve(90);
	ExitNodeList.Append(StartData.ExitNodes);

	const int32 ActualBossCount = BossList.Num();
	const int32 TargetSpecialCount = 3 - ActualBossCount;
	const int32 TargetEscapeCount = TeleportPointAmount;
	int32 PlacedBossCount = 0;
	int32 PlacedSpecialCount = 0;
	int32 PlacedEscapeCount = 0;

	while (RoomAmount > 0 && !ExitNodeList.IsEmpty())
	{
		const int32 ExitIdx = RandomStream.RandRange(0, ExitNodeList.Num() - 1);
		UAZRoomExitNode* CurrentExitNode = ExitNodeList[ExitIdx];
		ExitNodeList.RemoveAtSwap(ExitIdx);

		if (!CurrentExitNode) continue;

		const FTransform ExitTransform = CurrentExitNode->ExitTransform;
		bool bPlaced = false;

		// Boss 우선 배치
		if (PlacedBossCount < ActualBossCount && CanPlaceBossRoom(CurrentExitNode))
		{
			if (TryPlaceRoom(RoomDataAsset->BossRoomData.RoomData, CurrentExitNode, ExitTransform))
			{
				RoomSpawnInfos.Last().bIsBossRoom = true;
				BossRoomIndices.Add(RoomSpawnInfos.Num() - 1);
				PlacedBossCount++;

				ExitNodeList.Append(RoomSpawnInfos.Last().ExitNodes);
				bPlaced = true;
			}
		}

		// 대체 룸 배치
		if (!bPlaced && PlacedBossCount >= ActualBossCount && PlacedSpecialCount < TargetSpecialCount && CanPlaceAlterRoom(CurrentExitNode))
		{
			if (TryPlaceRoom(RoomDataAsset->SpecialRoomData.RoomData, CurrentExitNode, ExitTransform))
			{
				PlacedSpecialCount++;
				ExitNodeList.Append(RoomSpawnInfos.Last().ExitNodes);
				bPlaced = true;
			}
		}

		// Normal 룸 배치
		if (!bPlaced)
		{
			const int32 PoolNum = RoomLevelAssets.Num();
			if (PoolNum == 0) break;

			TArray<int32> CandidateIndices;
			CandidateIndices.Reserve(PoolNum);
			for (int32 i = 0; i < PoolNum; ++i) CandidateIndices.Add(i);

			while (!CandidateIndices.IsEmpty() && !bPlaced)
			{
				const int32 PickIdx = RandomStream.RandRange(0, CandidateIndices.Num() - 1);
				const int32 RoomIdx = CandidateIndices[PickIdx];
				CandidateIndices.RemoveAtSwap(PickIdx);

				FSpawnLevelData& Candidate = RoomLevelAssets[RoomIdx];
				if (TryPlaceRoom(Candidate.RoomData, CurrentExitNode, ExitTransform))
				{
					if (--Candidate.Amount <= 0) RoomLevelAssets.RemoveAtSwap(RoomIdx);

					RoomAmount--;
					ExitNodeList.Append(RoomSpawnInfos.Last().ExitNodes);
					bPlaced = true;

					// 탈출 지점 배치 시도
					const int32 PlacedIndex = RoomSpawnInfos.Num() - 1;
					if (PlacedEscapeCount < TargetEscapeCount && PlacedIndex > 0 && CanAssignEscapePoint(PlacedIndex))
					{
						RoomSpawnInfos.Last().bIsEscapeRoom = true;
						EscapeRoomIndices.Add(PlacedIndex);
						PlacedEscapeCount++;

						UE_LOG(LogTemp, Log, TEXT("[SimulateMapGenerate] 탈출 지점 배치. 룸 %d"), PlacedIndex);
					}
				}
			}
		}
	}

	// 부족한 보스, 탈출 지점 강제 배치
	FinalizeBossRooms(ExitNodeList, PlacedBossCount, ActualBossCount);
	FinalizeEscapePoints(PlacedEscapeCount, TargetEscapeCount);

	return ValidateGeneratedMap(TargetTotalRooms, ActualBossCount, TargetEscapeCount);
}

void AAZRandomMapGenerator::CreateSimulationData(const FRoomData& RoomData, UAZRoomExitNode* ParentNode, const FTransform& Entrance)
{
	FSimulationData NewData;
	NewData.RoomIndex = RoomSpawnInfos.Num();
	NewData.ParentExitNode = ParentNode;
	NewData.RoomData = RoomData;

	// 시작 룸으로부터의 거리 계산
	if (ParentNode)
	{
		const int32 ParentIndex = ParentNode->OwnerIndex;
		if (RoomSpawnInfos.IsValidIndex(ParentIndex))
		{
			NewData.ParentRoomIndex = ParentIndex;
			NewData.DistanceFromStart = RoomSpawnInfos[ParentIndex].DistanceFromStart + 1;
			RoomSpawnInfos[ParentIndex].ChildIndices.Add(NewData.RoomIndex);
		}
	}

	for (const FTransform& LocalTransform : RoomData.LocalExitTransforms)
	{
		UAZRoomExitNode* ExitNode = NewObject<UAZRoomExitNode>(this);
		ExitNode->NodeID = NodeIdIdx++;
		ExitNode->OwnerIndex = NewData.RoomIndex;
		ExitNode->ExitTransform = LocalTransform * Entrance;
		NewData.ExitNodes.Add(ExitNode);
	}

	RoomSpawnInfos.Add(NewData);
}

void AAZRandomMapGenerator::FindRoomInfoById(int32 ParentID)
{
	if (!RoomSpawnInfos.IsValidIndex(SpawnIdx)) return;

	FSimulationData& SpawnRoom = RoomSpawnInfos[SpawnIdx];

	if (SpawnIdx == 0)
	{
		FActorSpawnParameters SpawnParam;
		SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AAZDoor* Door = GetWorld()->SpawnActor<AAZDoor>(DoorClass, GetTransform(), SpawnParam))
			Door->SetDoorTrigger(false);
	}

	RoomSpawn(SpawnRoom, SpawnIdx == 0 ? nullptr : SpawnRoom.ParentExitNode);
	++SpawnIdx;
}

void AAZRandomMapGenerator::RoomSpawn(FSimulationData& SpawnData, UAZRoomExitNode* EntranceNode)
{
	CurrentSpawnLevel.LevelAsset = SpawnData.RoomData.LevelAsset;
	CurrentSpawnLevel.RoomIdx = SpawnData.RoomIndex;
	CurrentSpawnLevel.ParentNode = SpawnData.ParentExitNode;

	if (SpawnData.ParentExitNode)
	{
		CurrentSpawnLevel.SpawnTransform = SpawnData.ParentExitNode->ExitTransform;
		CurrentSpawnLevel.ParentRoom = RoomSpawnInfos[SpawnData.ParentExitNode->OwnerIndex].SpawnedRoom;
	}
	else
	{
		CurrentSpawnLevel.SpawnTransform = FTransform::Identity;
		CurrentSpawnLevel.ParentRoom = nullptr;
	}

	bool bSuccess = false;
	FString InstanceName = FString::Printf(TEXT("Room_Inst_%d"), SpawnIdx);

	CurrentSpawnLevel.StreamingHandle = ULevelStreamingDynamic::LoadLevelInstance(
		this,
		CurrentSpawnLevel.LevelAsset.GetLongPackageName(),
		CurrentSpawnLevel.SpawnTransform.GetLocation(),
		CurrentSpawnLevel.SpawnTransform.GetRotation().Rotator(),
		bSuccess,
		InstanceName
	);

	if (bSuccess && CurrentSpawnLevel.StreamingHandle)
	{
		if (SpawnData.ParentExitNode)
		{
			DoorList.Remove(SpawnData.ParentExitNode->AttachedDoor);

			if (CurrentSpawnLevel.LevelAsset == RoomDataAsset->BossRoomData.RoomData.LevelAsset)
				CurrentSpawnLevel.EntranceDoor = SpawnData.ParentExitNode->AttachedDoor;
		}
		CurrentSpawnLevel.StreamingHandle->OnLevelShown.AddDynamic(this, &AAZRandomMapGenerator::OnRoomLevelSpawned);
	}
	else
	{
		GenerateContents();
	}
}

void AAZRandomMapGenerator::OnRoomLevelSpawned()
{
	if (!CurrentSpawnLevel.ValidCheck()) return;
	CurrentSpawnLevel.StreamingHandle->OnLevelShown.RemoveDynamic(this, &AAZRandomMapGenerator::OnRoomLevelSpawned);

	const FRoomSpawnContext CompletedContext = CurrentSpawnLevel;
	CurrentSpawnLevel.ResetContext();

	ULevel* LoadedLevel = CompletedContext.StreamingHandle->GetLoadedLevel();
	if (!LoadedLevel)
	{
		GenerateContents();
		return;
	}

	AAZBaseRoom* SpawnedRoom = FindSpawnedRoomInLevel(LoadedLevel);
	if (!IsValid(SpawnedRoom)) return;

	SpawnedRoom->PersistentWorld = GetWorld();
	SpawnedRoom->StreamingHandle = CompletedContext.StreamingHandle;
	RoomSpawnInfos[CompletedContext.RoomIdx].SpawnedRoom = SpawnedRoom;

	LinkDoors(SpawnedRoom, CompletedContext);
	LinkAdjacentRoom(SpawnedRoom, CompletedContext);
	CollectRoomInfo(SpawnedRoom, CompletedContext.RoomIdx);

	if (SpawnIdx < RoomSpawnInfos.Num())
		FindRoomInfoById(CompletedContext.RoomIdx);
	else
		GenerateContents();
}

AAZBaseRoom* AAZRandomMapGenerator::FindSpawnedRoomInLevel(ULevel* Level) const
{
	if (!Level) return nullptr;

	for (AActor* Actor : Level->Actors)
	{
		if (AAZBaseRoom* SpawnedRoom = Cast<AAZBaseRoom>(Actor))
			return SpawnedRoom;
	}

	return nullptr;
}

void AAZRandomMapGenerator::LinkDoors(AAZBaseRoom* Room, const FRoomSpawnContext& Context)
{
	if (!Room || !Room->ExitPoints) return;

	TArray<USceneComponent*> ChildrenComp;
	Room->ExitPoints->GetChildrenComponents(false, ChildrenComp);

	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const auto& ExitNodes = RoomSpawnInfos[Context.RoomIdx].ExitNodes;

	constexpr float Tolerance = 1.0f;

	for (USceneComponent* Child : ChildrenComp)
	{
		if (Child->GetNumChildrenComponents() == 0) continue;

		USceneComponent* DoorComponent = Child->GetChildComponent(0);
		if (!DoorComponent) continue;

		const FTransform DoorTransform = DoorComponent->GetComponentTransform();

		if (UChildActorComponent* ChildActorComp = Cast<UChildActorComponent>(DoorComponent))
			ChildActorComp->DestroyComponent();

		AAZDoor* Door = GetWorld()->SpawnActor<AAZDoor>(DoorClass, DoorTransform, SpawnParam);
		if (!Door) continue;

		DoorList.Add(Door);
		const FVector DoorLoc = Door->GetActorLocation();

		for (UAZRoomExitNode* Node : ExitNodes)
		{
			if (!Node || IsValid(Node->AttachedDoor)) continue;

			// 위치 비교
			if (FVector::DistSquared(Node->ExitTransform.GetLocation(), DoorLoc) <= Tolerance)
			{
				Node->AttachedDoor = Door;
				break;
			}
		}
	}
}

void AAZRandomMapGenerator::LinkAdjacentRoom(AAZBaseRoom* Room, const FRoomSpawnContext& Context)
{
	if (!Room) return;

	if (!Context.ParentRoom)
	{
		Room->RoomType = ERoomType::Start;
		Room->EmptySpawnPoint();
		StartRoom = Room;
		return;
	}

	Context.ParentRoom->AdjacentRooms.AddUnique(Room);
	Room->AdjacentRooms.AddUnique(Context.ParentRoom);
	if (!bIsSimulate) Room->ActivateRoom(false);

	const bool bIsBossLevel = (Context.LevelAsset == RoomDataAsset->BossRoomData.RoomData.LevelAsset);

	if (bIsBossLevel && !BossList.IsEmpty())
	{
		Room->RoomType = ERoomType::Boss;

		int32 Idx;
		EBossType Type;
		if (GetRandomArrayItemFromStream(BossList, RandomStream, Type, Idx) && BossList.IsValidIndex(Idx))
		{
			BossList.RemoveAt(Idx);
			Room->BossType = Type;

			if (Context.EntranceDoor)
				Room->OnBossFight.AddDynamic(Context.EntranceDoor, &AAZDoor::OnDoorActive);
		}
	}
}

void AAZRandomMapGenerator::CloseDoors()
{
	for (AAZDoor* Door : DoorList)
		if(IsValid(Door)) Door->SetDoorTrigger(false);
}

void AAZRandomMapGenerator::GenerateContents()
{
	UE_LOG(LogTemp, Warning, TEXT("GenerateContents"));

	CloseDoors();

	if (HasAuthority())
	{
		while (TreasureAmount > 0)      SpawnTreasureBox();
		while (TeleportPointAmount > 0) SpawnTeleporter();

		UpdateNavMeshBounds();

		if (!bIsSimulate && IsValid(GetWorld()->GetAuthGameMode<AAZStageGameMode>()))
		{
			FActorSpawnParameters SpawnParam;
			SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), StartPoint, SpawnParam);
		}
	}

	if (UAZSoundManagerSubsystem* SoundSystem = GetGameInstance()->GetSubsystem<UAZSoundManagerSubsystem>())
		SoundSystem->PlayBGM(EBGMType::BGM_Field);

	if (UAZSessionSubsystem* SessionSystem = GetGameInstance()->GetSubsystem<UAZSessionSubsystem>())
		SessionSystem->RandomSeed = DeterminedSeed;

	if (AAZPlayerController* PC = Cast<AAZPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		PC->OnMapGenerationFinished();
		UE_LOG(LogTemp, Warning, TEXT("MapGenerate Finish"));
	}
}

void AAZRandomMapGenerator::SpawnTreasureBox()
{
	if (!TreasureClass || TreasureList.IsEmpty())
	{
		TreasureAmount = 0;
		return;
	}

	int32 Index;
	FTransform SpawnPoint;
	if (!GetRandomArrayItemFromStream(TreasureList, RandomStream, SpawnPoint, Index)) return;
	TreasureList.RemoveAt(Index);

	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(TreasureClass, SpawnPoint, SpawnParam);

	--TreasureAmount;
}

void AAZRandomMapGenerator::SpawnTeleporter()
{
	if (TeleporterList.IsEmpty() || !TeleporterClass)
	{
		TeleportPointAmount = 0;
		return;
	}

	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(TeleporterClass, TeleporterList.Top(), SpawnParam);
	TeleporterList.Pop();
	--TeleportPointAmount;
}

void AAZRandomMapGenerator::UpdateNavMeshBounds()
{
	if (MinX == TNumericLimits<float>::Max()) return;

	FVector MinPos(MinX, MinY, 0.f);
	FVector MaxPos(MaxX, MaxY, 0.f);
	FVector Center = (MinPos + MaxPos) * 0.5f;
	FVector Extent = (MaxPos - MinPos) / 100.f; // UU -> m 스케일 변환
	Extent.Z = 1.f;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANavMeshBoundsVolume::StaticClass(), FoundActors);
	if (FoundActors.IsEmpty()) return;

	ANavMeshBoundsVolume* NavMeshVolume = Cast<ANavMeshBoundsVolume>(FoundActors[0]);
	if (!NavMeshVolume) return;

	NavMeshVolume->GetBrushComponent()->SetMobility(EComponentMobility::Movable);
	NavMeshVolume->SetActorLocation(Center);
	NavMeshVolume->SetActorScale3D(Extent);

	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		NavSys->OnNavigationBoundsUpdated(NavMeshVolume);
		NavSys->Build();
	}
}

void AAZRandomMapGenerator::CollectRoomInfo(AAZBaseRoom* SpawnedRoom, int32 RoomIdx)
{
	if (!IsValid(SpawnedRoom)) return;

	if (!bSpawnedStartLevel)
	{
		StartPoint = SpawnedRoom->StartPoint->GetComponentTransform();
		bSpawnedStartLevel = true;
	}

	if (SpawnedRoom->OverlapTrigger)
	{
		const FVector Center = SpawnedRoom->OverlapTrigger->GetComponentLocation();
		const FVector Extent = SpawnedRoom->OverlapTrigger->GetScaledBoxExtent();

		MinX = FMath::Min(MinX, Center.X - Extent.X);
		MaxX = FMath::Max(MaxX, Center.X + Extent.X);
		MinY = FMath::Min(MinY, Center.Y - Extent.Y);
		MaxY = FMath::Max(MaxY, Center.Y + Extent.Y);
	}

	TArray<USceneComponent*> OutItemPoints;
	SpawnedRoom->ItemSpawnPoints->GetChildrenComponents(false, OutItemPoints);
	for (USceneComponent* ItemPoint : OutItemPoints)
	{
		TreasureList.Emplace(ItemPoint->GetComponentTransform());
	}

	if (RoomSpawnInfos.IsValidIndex(RoomIdx) && RoomSpawnInfos[RoomIdx].bIsEscapeRoom)
	{
		TArray<USceneComponent*> OutTeleportPoints;
		SpawnedRoom->TeleportPoint->GetChildrenComponents(false, OutTeleportPoints);
		for (USceneComponent* TeleportPoint : OutTeleportPoints)
			TeleporterList.Emplace(TeleportPoint->GetComponentTransform());
	}
}

bool AAZRandomMapGenerator::TryPlaceRoom(const FRoomData& RoomData, UAZRoomExitNode* ParentNode, const FTransform& ExitTransform)
{
	const FVector WorldOffset = ExitTransform.GetLocation() + ExitTransform.Rotator().RotateVector(RoomData.BoundsOffset);

	if (!CheckRoomAABB(RoomData, WorldOffset, ExitTransform)) return false;

	const FBox LocalBox(-RoomData.BoundsExtent, RoomData.BoundsExtent);
	const FTransform BoxTransform(ExitTransform.GetRotation(), WorldOffset);
	PlacedBounds.Add(LocalBox.TransformBy(BoxTransform));

	CreateSimulationData(RoomData, ParentNode, ExitTransform);
	return true;
}

bool AAZRandomMapGenerator::CheckRoomAABB(const FRoomData& RoomData, const FVector& WorldOffset, const FTransform& ExitTransform)
{
	const FBox LocalBox(-RoomData.BoundsExtent, RoomData.BoundsExtent);
	const FTransform BoxTransform(ExitTransform.GetRotation(), WorldOffset);
	const FBox NewAABB = LocalBox.TransformBy(BoxTransform);

	constexpr float Tolerance = 0.05f;
	for (const FBox& PlacedAABB : PlacedBounds)
	{
		// FBox::Intersect는 > 비교이기 때문에 값이 0(완전히 맞닿음)이어도 충돌 판정
		const bool bSeparated =
			(NewAABB.Min.X >= PlacedAABB.Max.X - Tolerance) ||
			(PlacedAABB.Min.X >= NewAABB.Max.X - Tolerance) ||
			(NewAABB.Min.Y >= PlacedAABB.Max.Y - Tolerance) ||
			(PlacedAABB.Min.Y >= NewAABB.Max.Y - Tolerance) ||
			(NewAABB.Min.Z >= PlacedAABB.Max.Z - Tolerance) ||
			(PlacedAABB.Min.Z >= NewAABB.Max.Z - Tolerance);

		if (!bSeparated) return false;
	}

	return true;
}

// 보스 룸 배치 가능 여부 확인
bool AAZRandomMapGenerator::CanPlaceBossRoom(UAZRoomExitNode* ParentExitNode)
{
	if (!ParentExitNode) return false;

	const int32 ParentRoomIndex = ParentExitNode->OwnerIndex;
	if (!RoomSpawnInfos.IsValidIndex(ParentRoomIndex)) return false;

	const int32 PredictedDistance = RoomSpawnInfos[ParentRoomIndex].DistanceFromStart + 1;
	if (PredictedDistance < MinSpecialRoomDistance) return false;

	// 다른 보스 룸들로부터의 거리 확인 (4칸 이상)
	for (int32 BossRoomIndex : BossRoomIndices)
	{
		const int32 Distance = CalculateRoomDistance(BossRoomIndex, ParentRoomIndex) + 1;
		if (Distance >= 0 && Distance < MinSpecialRoomDistance) return false;
	}

	// 탈출 지점 룸과의 거리
	for (int32 EscapeIdx : EscapeRoomIndices)
	{
		const int32 Distance = CalculateRoomDistance(EscapeIdx, ParentRoomIndex) + 1;
		if (Distance >= 0 && Distance < MinSpecialRoomDistance) return false;
	}

	return true;
}

bool AAZRandomMapGenerator::CanPlaceAlterRoom(UAZRoomExitNode* ParentExitNode)
{
	if (!ParentExitNode) return false;

	const int32 ParentRoomIndex = ParentExitNode->OwnerIndex;
	if (!RoomSpawnInfos.IsValidIndex(ParentRoomIndex)) return false;

	// 스페셜 룸은 일반 룸이지만, 다른 보스 룸 바로 옆에 연달아 배치되는 것은 방지 (최소 거리 2)
	for (int32 BossRoomIndex : BossRoomIndices)
	{
		const int32 Distance = CalculateRoomDistance(BossRoomIndex, ParentRoomIndex) + 1;
		if (Distance >= 0 && Distance < 2) return false;
	}

	return true;
}

void AAZRandomMapGenerator::FinalizeBossRooms(TArray<UAZRoomExitNode*>& ExitNodeList, int32 PlacedBossCount, int32 TargetBossCount)
{
	// 남은 보스 룸 배치
	while (PlacedBossCount < TargetBossCount && !ExitNodeList.IsEmpty())
	{
		bool bPlaced = false;
		for (int32 i = ExitNodeList.Num() - 1; i >= 0; --i)
		{
			UAZRoomExitNode* ExitNode = ExitNodeList[i];
			if (!CanPlaceBossRoom(ExitNode)) continue;

			if (TryPlaceRoom(RoomDataAsset->BossRoomData.RoomData, ExitNode, ExitNode->ExitTransform))
			{
				RoomSpawnInfos.Last().bIsBossRoom = true;
				BossRoomIndices.Add(RoomSpawnInfos.Num() - 1);
				PlacedBossCount++;
				ExitNodeList.RemoveAt(i);
				ExitNodeList.Append(RoomSpawnInfos.Last().ExitNodes);
				bPlaced = true;
				break;
			}
		}

		if (!bPlaced)
		{
			UE_LOG(LogTemp, Warning, TEXT("보스 룸 배치 실패. 배치 수: %d, 목표 수: %d"), PlacedBossCount, TargetBossCount);
			break;
		}
	}
}

bool AAZRandomMapGenerator::CanAssignEscapePoint(int32 RoomIndex) const
{
	if (!RoomSpawnInfos.IsValidIndex(RoomIndex)) return false;

	const FSimulationData& Room = RoomSpawnInfos[RoomIndex];

	if (RoomIndex == 0) return false;
	if (Room.bIsBossRoom) return false;
	if (Room.bIsEscapeRoom) return false;

	if (Room.DistanceFromStart < MinSpecialRoomDistance) return false;

	for (int32 BossIdx : BossRoomIndices)
	{
		const int32 Dist = CalculateRoomDistance(RoomIndex, BossIdx);
		if (Dist >= 0 && Dist < MinSpecialRoomDistance) return false;
	}

	for (int32 EscapeIdx : EscapeRoomIndices)
	{
		const int32 Dist = CalculateRoomDistance(RoomIndex, EscapeIdx);
		if (Dist >= 0 && Dist < MinSpecialRoomDistance) return false;
	}

	return true;
}

void AAZRandomMapGenerator::FinalizeEscapePoints(int32 PlacedEscapeCount, int32 TargetEscapeCount)
{
	if (PlacedEscapeCount >= TargetEscapeCount) return;
	UE_LOG(LogTemp, Warning, TEXT("FinalizeEscapePoints"));

	// 모든 일반 룸 인덱스를 시작 거리 역순으로 정렬 (먼 곳부터 배정)
	TArray<int32> Candidates;
	for (int32 i = 1; i < RoomSpawnInfos.Num(); ++i) // 시작 룸(0) 제외
	{
		const FSimulationData& Room = RoomSpawnInfos[i];
		if (!Room.bIsBossRoom && !Room.bIsEscapeRoom) Candidates.Add(i);
	}

	// 시작 룸에서 먼 순서로 정렬 (거리 제약 만족 확률 높음)
	Candidates.Sort([this](int32 A, int32 B)
		{
			return RoomSpawnInfos[A].DistanceFromStart > RoomSpawnInfos[B].DistanceFromStart;
		});

	for (int32 CandIdx : Candidates)
	{
		if (PlacedEscapeCount >= TargetEscapeCount) break;

		if (CanAssignEscapePoint(CandIdx))
		{
			RoomSpawnInfos[CandIdx].bIsEscapeRoom = true;
			EscapeRoomIndices.Add(CandIdx);
			PlacedEscapeCount++;
		}
	}
}

int32 AAZRandomMapGenerator::CalculateRoomDistance(int32 FromRoomIndex, int32 ToRoomIndex) const
{
	if (FromRoomIndex == ToRoomIndex) return 0;
	if (!RoomSpawnInfos.IsValidIndex(FromRoomIndex) || !RoomSpawnInfos.IsValidIndex(ToRoomIndex))
		return -1;

	// BFS (양방향 탐색)
	TQueue<int32> Queue;
	TMap<int32, int32> Distances;
	TSet<int32> Visited;

	Queue.Enqueue(FromRoomIndex);
	Distances.Add(FromRoomIndex, 0);
	Visited.Add(FromRoomIndex);

	while (!Queue.IsEmpty())
	{
		int32 CurrentRoom;
		Queue.Dequeue(CurrentRoom);

		if (CurrentRoom == ToRoomIndex)	return Distances[CurrentRoom];

		const int32 CurrentDist = Distances[CurrentRoom];

		// 부모 룸 탐색
		if (RoomSpawnInfos[CurrentRoom].ParentExitNode)
		{
			int32 ParentIndex = RoomSpawnInfos[CurrentRoom].ParentExitNode->OwnerIndex;
			if (!Visited.Contains(ParentIndex))
			{
				Visited.Add(ParentIndex);
				Distances.Add(ParentIndex, CurrentDist + 1);
				Queue.Enqueue(ParentIndex);
			}
		}

		// 자식 룸 탐색
		for (int32 ChildIndex : RoomSpawnInfos[CurrentRoom].ChildIndices)
		{
			if (!Visited.Contains(ChildIndex))
			{
				Visited.Add(ChildIndex);
				Distances.Add(ChildIndex, CurrentDist + 1);
				Queue.Enqueue(ChildIndex);
			}
		}
	}

	return -1; // 연결되지 않음
}

bool AAZRandomMapGenerator::ValidateGeneratedMap(int32 TargetRoomCount, int32 TargetBossCount, int32 TargetEscapeCount) const
{
	bool bValid = true;

	const int32 ActualRoomCount = RoomSpawnInfos.Num();
	if (ActualRoomCount < TargetRoomCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Validate] 룸: %d / %d"), ActualRoomCount, TargetRoomCount);
		bValid = false;
	}

	if (BossRoomIndices.Num() < TargetBossCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Validate] 보스룸: %d / %d"), BossRoomIndices.Num(), TargetBossCount);
		bValid = false;
	}

	if (EscapeRoomIndices.Num() < TargetEscapeCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Validate] 탈출 지점: %d / %d"), EscapeRoomIndices.Num(), TargetEscapeCount);
		bValid = false;
	}

	// 보스 간 거리 검증
	for (int32 i = 0; i < BossRoomIndices.Num(); ++i)
	{
		const int32 BossIdx = BossRoomIndices[i];

		if (RoomSpawnInfos.IsValidIndex(BossIdx) && RoomSpawnInfos[BossIdx].DistanceFromStart < MinSpecialRoomDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Validate] 보스 %d이 시작 지점과 너무 가까움(거리 %d)"), BossIdx, RoomSpawnInfos[BossIdx].DistanceFromStart);
			bValid = false;
		}

		for (int32 j = i + 1; j < BossRoomIndices.Num(); ++j)
		{
			const int32 Dist = CalculateRoomDistance(BossIdx, BossRoomIndices[j]);
			if (Dist >= 0 && Dist < MinSpecialRoomDistance)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Validate] 보스 %d과 보스 %d가 너무 가까움(거리 %d)"), BossIdx, BossRoomIndices[j], Dist);
				bValid = false;
			}
		}

		for (int32 EscapeIdx : EscapeRoomIndices)
		{
			const int32 Dist = CalculateRoomDistance(BossIdx, EscapeIdx);
			if (Dist >= 0 && Dist < MinSpecialRoomDistance)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Validate] 보스 %d과 탈출 지점 %d이 너무 가까움(거리 %d)"), BossIdx, EscapeIdx, Dist);
				bValid = false;
			}
		}
	}

	// 탈출 지점 간 거리 검증
	for (int32 i = 0; i < EscapeRoomIndices.Num(); ++i)
	{
		for (int32 j = i + 1; j < EscapeRoomIndices.Num(); ++j)
		{
			const int32 Dist = CalculateRoomDistance(EscapeRoomIndices[i], EscapeRoomIndices[j]);
			if (Dist >= 0 && Dist < MinSpecialRoomDistance)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Validate] 탈출 지점 %d이 탈출 지점 %d과 너무 가까움(거리 %d)"), EscapeRoomIndices[i], EscapeRoomIndices[j], Dist);
				bValid = false;
			}
		}
	}

	if (bValid)
	{
		UE_LOG(LogTemp, Log, TEXT("[Validate] 검증 성공. Rooms:%d Boss:%d Escape:%d"), ActualRoomCount, BossRoomIndices.Num(), EscapeRoomIndices.Num());
	}

	return bValid;
}

void AAZRandomMapGenerator::ResetGenerationState()
{
	SpawnIdx = 0;
	NodeIdIdx = 0;
	bSpawnedStartLevel = false;
	StartRoom = nullptr;
	StartPoint = FTransform::Identity;

	MinX = TNumericLimits<float>::Max();
	MinY = TNumericLimits<float>::Max();
	MaxX = TNumericLimits<float>::Lowest();
	MaxY = TNumericLimits<float>::Lowest();

	RoomSpawnInfos.Reset();
	BossRoomIndices.Reset();
	EscapeRoomIndices.Reset();
	PlacedBounds.Reset();
	DoorList.Reset();
	TreasureList.Reset();
	TeleporterList.Reset();

	for (UNiagaraComponent* Niagara : NiagaraList)
		if (IsValid(Niagara)) Niagara->DestroyComponent();
	NiagaraList.Reset();

	CurrentSpawnLevel.ResetContext();
}

void AAZRandomMapGenerator::CreateSimulationBox(const FVector& Location, const FRotator& Rotation, const FVector& Extent)
{
	FVector NewScale = Extent * 2.0f;
	FVector2D SpriteSize = FVector2D(NewScale.X / 100, NewScale.X / 100);

	if (UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SimulationCube, Location, Rotation))
	{
		NiagaraComp->SetVariableVec3(TEXT("Scale"), NewScale);
		NiagaraComp->SetVariableVec2(TEXT("SpriteSize"), SpriteSize);
		NiagaraList.Add(NiagaraComp);
	}
}