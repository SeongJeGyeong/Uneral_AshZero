// Fill out your copyright notice in the Description page of Project Settings.


#include "Levels/AZSubLevelReserveBound.h"
#include "Components/BoxComponent.h"

// Sets default values
AAZSubLevelReserveBound::AAZSubLevelReserveBound()
{
	PrimaryActorTick.bCanEverTick = false;

    ReserveBound = CreateDefaultSubobject<UBoxComponent>(TEXT("ReserveBound"));
    RootComponent = ReserveBound;

    // 충돌 설정: 다른 방 생성 로직(Trace)을 막아야 함
    ReserveBound->SetCollisionProfileName(TEXT("RoomSweepCollision"));

    // 게임 중에는 보일 필요 없음 (디버그 때만 켜기)
    ReserveBound->SetHiddenInGame(false);
}

void AAZSubLevelReserveBound::InitializeReserver(FVector BoxExtent)
{
    ReserveBound->SetBoxExtent(BoxExtent);
}