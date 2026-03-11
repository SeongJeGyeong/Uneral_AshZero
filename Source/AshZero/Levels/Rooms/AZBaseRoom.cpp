// Fill out your copyright notice in the Description page of Project Settings.


#include "Levels/Rooms/AZBaseRoom.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Levels/Props/AZSpawnPointComponent.h"
#include "Levels/AZRandomMapGenerator.h"
#include "Character/AZPlayerCharacter.h"
#include "System/Subsystems/AZObjectPoolSubsystem.h"
#include "Engine/LevelStreamingDynamic.h"
#include "System/GameMode/AZStageGameMode.h"
#include "Enemy/AZEnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"    
#include "Engine/World.h"           


// Sets default values
AAZBaseRoom::AAZBaseRoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>("Root Scene");
	RootComponent = RootSceneComponent;

	RoomDirection = CreateDefaultSubobject<UArrowComponent>("Room Direction");
	RoomDirection->SetupAttachment(RootSceneComponent);
	RoomDirection->ArrowSize = 5.f;
	RoomDirection->ArrowLength = 100.f;

	StartPoint = CreateDefaultSubobject<UAZSpawnPointComponent>("Start Point");
	StartPoint->SetupAttachment(RootComponent);
	StartPoint->ArrowLength = 80.f;

	OverlapTrigger = CreateDefaultSubobject<UBoxComponent>("Overlap Trigger");
	OverlapTrigger->SetupAttachment(RootSceneComponent);
	OverlapTrigger->SetCollisionProfileName(TEXT("RoomSweepCollision"));

	ExitPoints = CreateDefaultSubobject<USceneComponent>("Exit Points");
	ExitPoints->SetupAttachment(RootComponent);
	ItemSpawnPoints = CreateDefaultSubobject<USceneComponent>("Item Spawn Points");
	ItemSpawnPoints->SetupAttachment(RootComponent);
	EnemySpawnPoints = CreateDefaultSubobject<USceneComponent>("Enemy Spawn Points");
	EnemySpawnPoints->SetupAttachment(RootComponent);
	TeleportPoint = CreateDefaultSubobject<USceneComponent>("Teleport Point");
	TeleportPoint->SetupAttachment(RootComponent);
	Props = CreateDefaultSubobject<USceneComponent>("Room Props");
	Props->SetupAttachment(RootComponent);

	bReplicates = true;
}

// Called when the game starts or when spawned
void AAZBaseRoom::BeginPlay()
{
	Super::BeginPlay();

	OverlapTrigger->OnComponentBeginOverlap.RemoveDynamic(this, &AAZBaseRoom::OnTriggerBeginOverlap);
	OverlapTrigger->OnComponentEndOverlap.RemoveDynamic(this, &AAZBaseRoom::OnTriggerEndOverlap);

	OverlapTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAZBaseRoom::OnTriggerBeginOverlap);
	OverlapTrigger->OnComponentEndOverlap.AddDynamic(this, &AAZBaseRoom::OnTriggerEndOverlap);
}

void AAZBaseRoom::ActivateRoom(bool bActive)
{
	if (HasAuthority() && !bActive)
	{
		DormantContainedEnemies();
	}

	// --- 서버: 룸 활성화 시, 레벨이 실제로 shown 된 직후에 Wake/Spawn 되도록 바인딩 ---
	if (HasAuthority() && bActive && StreamingHandle)
	{
		StreamingHandle->OnLevelShown.RemoveDynamic(this, &AAZBaseRoom::OnRoomLevelShown);
		StreamingHandle->OnLevelShown.AddDynamic(this, &AAZBaseRoom::OnRoomLevelShown);
	}

	if (StreamingHandle)
	{
		StreamingHandle->SetShouldBeVisible(bActive);
	}
	else
	{
		SetActorHiddenInGame(!bActive);
	}

	if (!PersistentWorld)
	{
		UE_LOG(LogTemp, Warning, TEXT("World is null"));
		return;
	}

	// 스트리밍이 아닌 경우(혹시 모를 케이스), 바로 처리
	if (HasAuthority() && bActive && !StreamingHandle)
	{
		OnRoomLevelShown();
	}
}

void AAZBaseRoom::EmptySpawnPoint()
{
	TArray<USceneComponent*> ItemChildren;
	ItemSpawnPoints->GetChildrenComponents(false, ItemChildren);
	for (USceneComponent* SpawnPoint : ItemChildren)
	{
		SpawnPoint->DestroyComponent();
	}
	TArray<USceneComponent*> EnemyChildren;
	EnemySpawnPoints->GetChildrenComponents(false, EnemyChildren);
	for (USceneComponent* SpawnPoint : EnemyChildren)
	{
		SpawnPoint->DestroyComponent();
	}
}

void AAZBaseRoom::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->IsA(AAZPlayerCharacter::StaticClass())) return;

	++PlayerCount;
	for (AAZBaseRoom* Room : AdjacentRooms)
	{
		if (IsValid(Room) && Room->PlayerCount <= 0)
			Room->ActivateRoom(true);
	}
}

void AAZBaseRoom::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(AAZPlayerCharacter::StaticClass()))
	{
		--PlayerCount;
		if (PlayerCount < 0) PlayerCount = 0;
		if (PlayerCount > 0) return;

		for (AAZBaseRoom* Room : AdjacentRooms)
		{
			bool bActive = (Room->PlayerCount > 0);
			if (!bActive)
			{
				for (AAZBaseRoom* LinkedRoom : Room->AdjacentRooms)
				{
					if (LinkedRoom->PlayerCount > 0)
					{
						bActive = true;
						break;
					}
				}
			}

			Room->ActivateRoom(bActive);
		}
	}
}

void AAZBaseRoom::OnRoomLevelShown()
{
	if (!HasAuthority()) return;
	if (!PersistentWorld) return;

	// 1) Dormant 해둔 몬스터 깨우기
	WakeDormantEnemies();

	// 2) 기존 로직(룸 활성화 시 몬스터 스폰) - "레벨이 실제로 보이게 된 뒤" 실행
	if (RoomType == ERoomType::Boss || bEnemiesSpawned) return;

	UAZObjectPoolSubsystem* ObjectPool = PersistentWorld->GetSubsystem<UAZObjectPoolSubsystem>();
	if (!ObjectPool) return;
	if (ObjectPool->EnemyClassList.Num() <= 0) return;

	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TArray<USceneComponent*> ChildrenComp;
	EnemySpawnPoints->GetChildrenComponents(false, ChildrenComp);

	for (USceneComponent* SpawnPoint : ChildrenComp)
	{
		TSubclassOf<AAZEnemyBase> EnemyClass = ObjectPool->EnemyClassList[FMath::RandRange(0, ObjectPool->EnemyClassList.Num() - 1)];
		PersistentWorld->SpawnActor<AActor>(EnemyClass, SpawnPoint->GetComponentTransform(), SpawnParam);
	}

	bEnemiesSpawned = true;
}

void AAZBaseRoom::DormantContainedEnemies()
{
	if (!PersistentWorld || !OverlapTrigger) return;

	// Enemy 오브젝트 타입은 DefaultEngine.ini 기준으로 ECC_GameTraceChannel9 = "Enemy"
	// (Config에서 바뀌면 여기 채널도 맞춰야 함)
	const ECollisionChannel EnemyObjectType = ECC_GameTraceChannel9;

	const FVector BoxCenter = OverlapTrigger->GetComponentLocation();
	const FQuat BoxRot = OverlapTrigger->GetComponentQuat();
	const FVector BoxExtent = OverlapTrigger->GetScaledBoxExtent();

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(EnemyObjectType);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RoomDormant), false);
	QueryParams.AddIgnoredActor(this);

	TArray<FOverlapResult> Overlaps;
	const bool bHit = PersistentWorld->OverlapMultiByObjectType(
		Overlaps,
		BoxCenter,
		BoxRot,
		ObjParams,
		FCollisionShape::MakeBox(BoxExtent),
		QueryParams
	);

	if (!bHit) return;

	for (const FOverlapResult& Res : Overlaps)
	{
		AAZEnemyBase* Enemy = Cast<AAZEnemyBase>(Res.GetActor());
		if (!IsValid(Enemy)) continue;
		if (Enemy->bIsDead) continue;
		if (Enemy->bIsRoomDormant) continue;

		// "완전히 포함" 조건
		//if (!IsEnemyFullyInsideOverlapTrigger(Enemy)) continue;

		Enemy->SetRoomDormant(true);
		DormantEnemies.AddUnique(Enemy);
	}
}

void AAZBaseRoom::WakeDormantEnemies()
{
	// 죽었거나 파괴된 애들 정리 + Wake
	for (int32 i = DormantEnemies.Num() - 1; i >= 0; --i)
	{
		AAZEnemyBase* Enemy = DormantEnemies[i].Get();
		if (!IsValid(Enemy))
		{
			DormantEnemies.RemoveAtSwap(i);
			continue;
		}

		Enemy->SetRoomDormant(false);
		DormantEnemies.RemoveAtSwap(i);
	}
}

bool AAZBaseRoom::IsEnemyFullyInsideOverlapTrigger(const AAZEnemyBase* Enemy) const
{
	if (!OverlapTrigger || !IsValid(Enemy)) return false;

	const UCapsuleComponent* Capsule = Enemy->GetCapsuleComponent();
	if (!Capsule) return false;

	// OverlapTrigger의 OBB(회전 포함) 내부에 Capsule이 완전히 들어오는지 판정
	const FTransform BoxTM = OverlapTrigger->GetComponentTransform();
	const FVector BoxCenter = BoxTM.GetLocation();
	const FVector BoxExtent = OverlapTrigger->GetScaledBoxExtent();

	const FVector CapsuleCenter = Capsule->GetComponentLocation();
	const FVector D = CapsuleCenter - BoxCenter;

	// Capsule 파라미터(월드 스케일 반영)
	const float Radius = Capsule->GetScaledCapsuleRadius();
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float SegmentHalf = FMath::Max(0.f, HalfHeight - Radius);

	// Capsule 축(월드)
	const FVector CapsuleAxis = Capsule->GetUpVector().GetSafeNormal();

	const float Tol = 1.f;

	auto CheckAxis = [&](const FVector& Axis, float Extent) -> bool
		{
			const float CenterDist = FMath::Abs(FVector::DotProduct(D, Axis));
			const float AxisDot = FMath::Abs(FVector::DotProduct(CapsuleAxis, Axis));

			// Capsule = (선분 길이 2*SegmentHalf) ⊕ (반지름 Radius 구)
			// 특정 축 Axis 방향으로의 "반경" = SegmentHalf*|dot(CapsuleAxis,Axis)| + Radius
			const float Support = SegmentHalf * AxisDot + Radius;

			return (CenterDist + Support) <= (Extent + Tol);
		};

	const FVector AxisX = BoxTM.GetUnitAxis(EAxis::X);
	const FVector AxisY = BoxTM.GetUnitAxis(EAxis::Y);
	const FVector AxisZ = BoxTM.GetUnitAxis(EAxis::Z);

	return CheckAxis(AxisX, BoxExtent.X)
		&& CheckAxis(AxisY, BoxExtent.Y)
		&& CheckAxis(AxisZ, BoxExtent.Z);
}
