// Fill out your copyright notice in the Description page of Project Settings.


#include "Levels/Rooms/AZRoomBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAZRoomBase::AAZRoomBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Cube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomBlock"));
	SetRootComponent(Cube);
}

// Called when the game starts or when spawned
void AAZRoomBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAZRoomBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AAZRoomBase::IsSeparateOverlappingRooms()
{
	bool bFlag = true;
	TSet<AActor*> outRooms;
	GetOverlappingActors(outRooms, TSubclassOf<AAZRoomBase>());
	int32 count = 0;

	FVector pLoc(735.0, 260.0, 216.0);
	auto selfDistance = FVector::Distance(pLoc, GetActorLocation());

	for (AActor* room : outRooms)
	{
		bFlag = false; // if there are any rooms overlapping
		auto frndLoc = room->GetActorLocation();
		auto frndDistance = FVector::Distance(pLoc, frndLoc);
		auto toCheck = selfDistance < frndDistance ? frndLoc : GetActorLocation();
		auto dir = toCheck - pLoc;
		dir.Normalize();
		FVector offset = dir * 5;
		offset.Z = 0;

		offset = selfDistance < frndDistance ? offset : -offset;
		room->AddActorLocalOffset(offset);
		AddActorLocalOffset(-offset);
		count++;
	}

	return bFlag;
}

void AAZRoomBase::HighlightRoom()
{
	Cube->SetMaterial(0, OrangeMaterial);
}