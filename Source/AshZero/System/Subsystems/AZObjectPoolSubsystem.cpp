// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Subsystems/AZObjectPoolSubsystem.h"
#include "System/GameMode/AZStageGameMode.h"

UAZObjectPoolSubsystem::UAZObjectPoolSubsystem()
{
    static ConstructorHelpers::FClassFinder<AAZEnemyBase> Frog(TEXT("/Game/Blueprints/Enemy/Frog/BP_Frog.BP_Frog_C"));
    if (Frog.Succeeded())
    {
        EnemyClassList.Add(Frog.Class);
    }

    static ConstructorHelpers::FClassFinder<AAZEnemyBase> Robot(TEXT("/Game/Blueprints/Enemy/SM_121101/BP_121101.BP_121101_C"));
    if (Robot.Succeeded())
    {
        EnemyClassList.Add(Robot.Class);
    }

    static ConstructorHelpers::FClassFinder<AAZEnemyBase> Drone1(TEXT("/Game/Blueprints/Enemy/SM_111103/BP_111103.BP_111103_C"));
    if (Drone1.Succeeded())
    {
        EnemyClassList.Add(Drone1.Class);
    }

    static ConstructorHelpers::FClassFinder<AAZEnemyBase> Drone2(TEXT("/Game/Blueprints/Enemy/SM_111203/BP_111203.BP_111203_C"));
    if (Drone2.Succeeded())
    {
        EnemyClassList.Add(Drone2.Class);
    }

    static ConstructorHelpers::FClassFinder<AAZBossGigantia> SpineQueen(TEXT("/Game/Blueprints/Enemy/Gigantia/BP_SpineQueen.BP_SpineQueen_C"));
    if (SpineQueen.Succeeded())
    {
        SpineQueenClass = SpineQueen.Class;
    }

    static ConstructorHelpers::FClassFinder<AAZBossArgus> Argus(TEXT("/Game/Blueprints/Enemy/Argus/BP_Argus.BP_Argus_C"));
    if (Argus.Succeeded())
    {
        ArgusClass = Argus.Class;
    }

    static ConstructorHelpers::FClassFinder<AAZBossBase> Zephyros(TEXT("/Game/Blueprints/Enemy/Zephyros/BP_Zephyros.BP_Zephyros_C"));
    if (Zephyros.Succeeded())
    {
        ZephyrosClass = Zephyros.Class;
    }
}

AAZBossBase* UAZObjectPoolSubsystem::SpawnBoss(EBossType Type, const FTransform& SpawnTransform)
{
    FActorSpawnParameters SpawnParam;
    SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AAZBossBase* Boss = nullptr;
    switch (Type)
    {
    case EBossType::Argus:
        Boss = GetWorld()->SpawnActor<AAZBossBase>(ArgusClass, SpawnTransform, SpawnParam);
        break;
    case EBossType::SpineQueen:
        Boss = GetWorld()->SpawnActor<AAZBossBase>(SpineQueenClass, SpawnTransform, SpawnParam);
        break;
    case EBossType::Zephyros:
        Boss = GetWorld()->SpawnActor<AAZBossBase>(ZephyrosClass, SpawnTransform, SpawnParam);
        break;
    }
    return Boss;
}

bool UAZObjectPoolSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    UWorld* World = Cast<UWorld>(Outer);

    // 게임 월드인지 확인 (에디터 월드 제외)
    if (!World || !World->IsGameWorld()) return false;

    // 월드 초기화 시점에는 아직 GameMode 인스턴스가 생성되지 않았을 수 있으므로
    // GetWorldSettings()를 통해 설정된 GameMode 클래스를 확인
    TSubclassOf<AGameModeBase> GameModeClass = World->GetWorldSettings()->DefaultGameMode;

    if (GameModeClass && GameModeClass->IsChildOf(AAZStageGameMode::StaticClass()))
    {
        return true;
    }

    return false;
}

void UAZObjectPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}
