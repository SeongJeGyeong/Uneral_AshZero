// Fill out your copyright notice in the Description page of Project Settings.


#include "Levels/AZRandomMapGenerator.h"
#include "Levels/Rooms/AZBaseRoom.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Interactables/AZChest.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "System/GameMode/AZStageGameMode.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "GameFramework/SpectatorPawn.h"
#include "Components/BrushComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/LevelStreamingDynamic.h"
#include "AZSubLevelReserveBound.h"
#include "Levels/Props/AZDoor.h"
#include "System/Player/AZPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "DataAsset/AZRoomDataAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"
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
	ResetGenerationState();

	const int32 GenerateSeed = (bIsSimulate || Seed > 0) ? Seed : SeedValue;
	RandomStream.Initialize(GenerateSeed);

	UE_LOG(LogTemp, Warning, TEXT("[MapGen] Seed: %d (%s)"), GenerateSeed, bIsSimulate ? TEXT("Simulate") : (Seed > 0 ? TEXT("Custom") : TEXT("Network")));

	RoomLevelAssets = RoomDataAsset->RoomDataList;

	if (bIsSimulate)
	{
		const FVector SpawnLocation = FVector(0.f, 0.f, 25000.f);
		if (APawn* Spector = GetWorld()->SpawnActor<APawn>(SpectorClass, SpawnLocation, FRotator::ZeroRotator))
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
				PC->Possess(Spector);
		}
	}

	SimulateMapGenerate();
}

void AAZRandomMapGenerator::SimulateMapGenerate()
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
		UE_LOG(LogTemp, Error, TEXT("No start room candidate: need at least 2 exits"));
		return;
	}

	// 시작 룸 결정
	const int32 index = StartCandidates[RandomStream.RandRange(0, StartCandidates.Num() - 1)];
	FSpawnLevelData StartRoomData = RoomLevelAssets[index];

	if (--RoomLevelAssets[index].Amount <= 0)
		RoomLevelAssets.RemoveAtSwap(index);

	--RoomAmount;

	// 시작 룸 시뮬레이션 데이터 생성
	SpawnReserveBound(StartRoomData.RoomData, FTransform::Identity);

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

	// 확장용 Exit 리스트
	TArray<UAZRoomExitNode*> ExitNodeList;
	ExitNodeList.Reserve(256);
	ExitNodeList.Append(StartData.ExitNodes);

	// 보스/탈출 목표 수
	const int32 TargetBossCount = BossList.Num();
	const int32 TargetEscapeCount = TeleportPointAmount;
	int32 PlacedBossCount = 0;
	int32 PlacedEscapeCount = 0;

	// 메인 루프
	while (RoomAmount > 0 && !ExitNodeList.IsEmpty())
	{
		const int32 ExitIdx = RandomStream.RandRange(0, ExitNodeList.Num() - 1);
		UAZRoomExitNode* CurrentExitNode = ExitNodeList[ExitIdx];
		ExitNodeList.RemoveAtSwap(ExitIdx);

		if (!CurrentExitNode) continue;

		const FTransform ExitTransform = CurrentExitNode->ExitTransform;
		bool bPlaced = false;

		// Boss 우선 배치
		if (PlacedBossCount < TargetBossCount && CanPlaceBossRoom(CurrentExitNode))
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

		// Escape 배치 (보스 모두 배치 후)
		if (!bPlaced && PlacedBossCount >= TargetBossCount && PlacedEscapeCount < TargetEscapeCount)
		{
			if (CanPlaceEscapeRoom(CurrentExitNode))
			{
				if (TryPlaceRoom(RoomDataAsset->SpecialRoomData.RoomData, CurrentExitNode, ExitTransform))
				{
					RoomSpawnInfos.Last().bIsEscapeRoom = true;
					EscapeRoomIndices.Add(RoomSpawnInfos.Num() - 1);
					PlacedEscapeCount++;
					ExitNodeList.Append(RoomSpawnInfos.Last().ExitNodes);
					bPlaced = true;
				}
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

			while (!CandidateIndices.IsEmpty() > 0 && !bPlaced)
			{
				const int32 PickIdx = RandomStream.RandRange(0, CandidateIndices.Num() - 1);
				const int32 RoomIdx = CandidateIndices[PickIdx];
				CandidateIndices.RemoveAtSwap(PickIdx);

				FSpawnLevelData& Candidate = RoomLevelAssets[RoomIdx];
				if (TryPlaceRoom(Candidate.RoomData, CurrentExitNode, ExitTransform))
				{
					if (--Candidate.Amount <= 0)
						RoomLevelAssets.RemoveAtSwap(RoomIdx);

					RoomAmount--;
					ExitNodeList.Append(RoomSpawnInfos.Last().ExitNodes);
					bPlaced = true;
				}
			}
		}
	}

	// 부족한 Boss/Escape 강제 배치(기존 함수 활용하되 ExitNodeList.RemoveAtSwap 등으로 정리 권장)
	FinalizeSpecialRooms(ExitNodeList, PlacedBossCount, TargetBossCount, PlacedEscapeCount, TargetEscapeCount);

	// Reserve cleanup
	for (AActor* ReserveBound : ReserveList)
		if (IsValid(ReserveBound)) ReserveBound->Destroy();

	ReserveList.Reset();

	CreateRealMap(INDEX_NONE);
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

void AAZRandomMapGenerator::CreateRealMap(int32 ParentID)
{
	if (!RoomSpawnInfos.IsValidIndex(SpawnIdx)) return;

	FSimulationData& SpawnRoom = RoomSpawnInfos[SpawnIdx];

	if (SpawnIdx == 0)
	{
		FActorSpawnParameters SpawnParam;
		SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AAZDoor* Door = GetWorld()->SpawnActor<AAZDoor>(DoorClass, GetTransform(), SpawnParam);
		if (Door->IsValidLowLevel())
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
	CollectRoomInfo(SpawnedRoom);

	if (SpawnIdx < RoomSpawnInfos.Num())
		CreateRealMap(CompletedContext.RoomIdx);
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
		Door->SetDoorTrigger(false);
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

void AAZRandomMapGenerator::SpawnReserveBound(const FRoomData& RoomData, const FTransform& SpawnTransform)
{
	const FVector CenterLocation =
		SpawnTransform.GetLocation() +
		SpawnTransform.GetRotation().RotateVector(RoomData.BoundsOffset);

	FTransform BoxTransform;
	BoxTransform.SetLocation(CenterLocation);
	BoxTransform.SetRotation(SpawnTransform.GetRotation());
	BoxTransform.SetScale3D(FVector::OneVector);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AAZSubLevelReserveBound* Reserver = GetWorld()->SpawnActor<AAZSubLevelReserveBound>(AAZSubLevelReserveBound::StaticClass(), BoxTransform, SpawnParams))
	{
		Reserver->RoomName = RoomData.LevelAsset.GetAssetName();
		Reserver->InitializeReserver(RoomData.BoundsExtent);
		ReserveList.Add(Reserver);
	}
}

void AAZRandomMapGenerator::CollectRoomInfo(AAZBaseRoom* SpawnedRoom)
{
	if (!IsValid(SpawnedRoom)) return;

	UE_LOG(LogTemp, Warning, TEXT("%s: %s, [X: %f, Y: %f, Z: %f]"), 
		*GetNetModeString(), 
		*SpawnedRoom->StreamingHandle->GetName(),
		SpawnedRoom->GetActorLocation().X, SpawnedRoom->GetActorLocation().Y, SpawnedRoom->GetActorLocation().Z);

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

	TArray<USceneComponent*> OutTeleportPoints;
	SpawnedRoom->TeleportPoint->GetChildrenComponents(false, OutTeleportPoints);
	for (USceneComponent* TeleportPoint : OutTeleportPoints)
	{
		TeleporterList.Emplace(TeleportPoint->GetComponentTransform());
	}
}

bool AAZRandomMapGenerator::TryPlaceRoom(const FRoomData& RoomData, UAZRoomExitNode* ParentNode, const FTransform& ExitTransform)
{
	const FVector WorldOffset = ExitTransform.GetLocation() + ExitTransform.Rotator().RotateVector(RoomData.BoundsOffset);

	TArray<FHitResult> OutHits;
	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		WorldOffset,
		WorldOffset,
		ExitTransform.GetRotation(),
		ECollisionChannel::ECC_GameTraceChannel3,
		FCollisionShape::MakeBox(RoomData.BoundsExtent)
	);

	if (bHit)
	{
		for (const FHitResult& Hit : OutHits)
			if (Hit.PenetrationDepth > 0.1f) return false;
	}

	SpawnReserveBound(RoomData, ExitTransform);
	CreateSimulationData(RoomData, ParentNode, ExitTransform);
	return true;
}

// 보스 룸 배치 가능 여부 확인
bool AAZRandomMapGenerator::CanPlaceBossRoom(UAZRoomExitNode* ParentExitNode)
{
	if (!ParentExitNode) return false;

	const int32 ParentRoomIndex = ParentExitNode->OwnerIndex;
	if (!RoomSpawnInfos.IsValidIndex(ParentRoomIndex)) return false;

	const int32 PredictedDistance = RoomSpawnInfos[ParentRoomIndex].DistanceFromStart + 1;
	if (PredictedDistance < 4) return false;

	// 다른 보스 룸들로부터의 거리 확인 (4칸 이상)
	for (int32 BossRoomIndex : BossRoomIndices)
	{
		const int32 Distance = CalculateRoomDistance(BossRoomIndex, ParentRoomIndex) + 1;
		if (Distance >= 0 && Distance < 4) return false;
	}

	return true;
}

// 탈출 지점 배치 가능 여부 확인
bool AAZRandomMapGenerator::CanPlaceEscapeRoom(UAZRoomExitNode* ParentExitNode)
{
	if (!ParentExitNode) return false;

	int32 ParentRoomIndex = ParentExitNode->OwnerIndex;
	if (!RoomSpawnInfos.IsValidIndex(ParentRoomIndex)) return false;

	const int32 PredictedDistance = RoomSpawnInfos[ParentRoomIndex].DistanceFromStart + 1;
	if (PredictedDistance < 4) return false;

	// 모든 보스 룸으로부터의 거리 확인 (3칸 이상)
	for (int32 BossRoomIndex : BossRoomIndices)
	{
		int32 Distance = CalculateRoomDistance(BossRoomIndex, ParentRoomIndex) + 1;
		if (Distance >= 0 && Distance < 4) return false;
	}

	// 다른 탈출 지점들로부터의 거리 확인 (3칸 이상)
	for (int32 EscapeRoomIndex : EscapeRoomIndices)
	{
		int32 Distance = CalculateRoomDistance(EscapeRoomIndex, ParentRoomIndex) + 1;
		if (Distance >= 0 && Distance < 4) return false;
	}

	return true;
}

void AAZRandomMapGenerator::FinalizeSpecialRooms(TArray<UAZRoomExitNode*>& ExitNodeList, int32 PlacedBossCount, int32 TargetBossCount, int32 PlacedEscapeCount, int32 TargetEscapeCount)
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
			UE_LOG(LogTemp, Warning, TEXT("Failed to place boss room. Placed: %d, Target: %d"),
				PlacedBossCount, TargetBossCount);
			break;
		}
	}

	// 탈출 룸 강제 배치
	while (PlacedEscapeCount < TargetEscapeCount && !ExitNodeList.IsEmpty())
	{
		bool bPlaced = false;
		for (int32 i = ExitNodeList.Num() - 1; i >= 0; --i)
		{
			UAZRoomExitNode* ExitNode = ExitNodeList[i];
			if (!CanPlaceEscapeRoom(ExitNode)) continue;

			if (TryPlaceRoom(RoomDataAsset->SpecialRoomData.RoomData, ExitNode, ExitNode->ExitTransform))
			{
				RoomSpawnInfos.Last().bIsEscapeRoom = true;
				EscapeRoomIndices.Add(RoomSpawnInfos.Num() - 1);
				PlacedEscapeCount++;
				ExitNodeList.RemoveAt(i);
				ExitNodeList.Append(RoomSpawnInfos.Last().ExitNodes);
				bPlaced = true;
				break;
			}
		}

		if (!bPlaced)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MapGen] Failed to force-place escape room. Placed: %d / %d"),
				PlacedEscapeCount, TargetEscapeCount);
			break;
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

		// 부모 룸 탐색
		if (RoomSpawnInfos[CurrentRoom].ParentExitNode)
		{
			int32 ParentIndex = RoomSpawnInfos[CurrentRoom].ParentExitNode->OwnerIndex;
			if (!Visited.Contains(ParentIndex))
			{
				Visited.Add(ParentIndex);
				Distances.Add(ParentIndex, Distances[CurrentRoom] + 1);
				Queue.Enqueue(ParentIndex);
			}
		}

		// 자식 룸 탐색
		for (int32 i = 0; i < RoomSpawnInfos.Num(); ++i)
		{
			if (Visited.Contains(i)) continue;

			if (RoomSpawnInfos[i].ParentExitNode &&
				RoomSpawnInfos[i].ParentExitNode->OwnerIndex == CurrentRoom)
			{
				Visited.Add(i);
				Distances.Add(i, Distances[CurrentRoom] + 1);
				Queue.Enqueue(i);
			}
		}
	}

	return -1; // 연결되지 않음
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
	DoorList.Reset();
	TreasureList.Reset();
	TeleporterList.Reset();

	for (AActor* Reserve : ReserveList)
		if (IsValid(Reserve)) Reserve->Destroy();
	ReserveList.Reset();

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

FString AAZRandomMapGenerator::GetNetModeString() const
{
	FString ModeMsg;
	switch (GetWorld()->GetNetMode())
	{
	case NM_Client:          return TEXT("Client");
	case NM_DedicatedServer: return TEXT("DedicatedServer");
	case NM_ListenServer:    return TEXT("ListenServer");
	default:                 return TEXT("Standalone");
	}
	return ModeMsg;
}
