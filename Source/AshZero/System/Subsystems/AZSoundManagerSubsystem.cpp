// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Subsystems/AZSoundManagerSubsystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/AZSoundDataAsset.h"
#include "DataAsset/AZUISoundDataAsset.h"
#include "Components/AudioComponent.h"
#include "System/Settings/AZDeveloperSettings.h"
#include "Character/AZPlayerCharacter.h"
#include "Enemy/AZEnemyBase.h"
#include "Enemy/AZBossBase.h"
#include "Components/CapsuleComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Levels/Props/AZDoor.h"

bool UAZSoundManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    TArray<UClass*> ChildClasses;
    GetDerivedClasses(GetClass(), ChildClasses, false);
    return (ChildClasses.Num() == 0); // 최종 자식 클래스만 생성
}

void UAZSoundManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UAZDeveloperSettings* Settings = GetDefault<UAZDeveloperSettings>();
    if (!Settings) return;

    SoundDataAsset = Settings->SoundMap.LoadSynchronous();
    UISoundDataAsset = Settings->UISoundDataTable.LoadSynchronous();
}

void UAZSoundManagerSubsystem::PlayBGM(EBGMType Type)
{
    if (!SoundDataAsset) return;
    TObjectPtr<USoundBase> Sound = SoundDataAsset->BGMList.FindRef(Type);
    if (Sound)
    {
        if (!MusicAudioComponent)
        {
            MusicAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), Sound);
            if (MusicAudioComponent)
            {
                MusicAudioComponent->bAutoDestroy = false;
            }
        }
        else
        {
            MusicAudioComponent->SetSound(Sound);
            MusicAudioComponent->Play();
        }
    }
}

void UAZSoundManagerSubsystem::StopBGM()
{
    MusicAudioComponent->Stop();
}

void UAZSoundManagerSubsystem::PlayFootstepSFX(AActor* Actor)
{
    if (!Actor || !SoundDataAsset) return;

    FVector FootLocation;
    ESurfaceType SurfaceType;

    if (!TraceFootSurface(Actor, FootLocation, SurfaceType)) return;

    EActorType ActorType = EActorType::A_Player;

    if (Actor->IsA(AAZBossBase::StaticClass()))
    {
        ActorType = EActorType::A_Boss;
    }
    else if (Actor->IsA(AAZEnemyBase::StaticClass()))
    {
        ActorType = EActorType::A_Enemy;
    }

    USoundBase* Sound = SoundDataAsset->GetFootStepSound(ActorType, SurfaceType);

    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, FootLocation);
    }
}

void UAZSoundManagerSubsystem::PlaySFX(AActor* Actor, ESFXType SFXType)
{
    if (!Actor || !SoundDataAsset) return;

    EActorType ActorType = EActorType::A_Player;

    if (Actor->IsA(AAZBossBase::StaticClass()))
    {
        ActorType = EActorType::A_Boss;
    }
    else if (Actor->IsA(AAZEnemyBase::StaticClass()))
    {
        ActorType = EActorType::A_Enemy;
    }
    else if (Actor->IsA(AAZDoor::StaticClass()))
    {
        ActorType = EActorType::A_Door;
    }

    USoundBase* Sound = SoundDataAsset->GetSound(ActorType, SFXType);

    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Actor->GetTargetLocation());
    }
}

void UAZSoundManagerSubsystem::PlayUISFX(EUISFXType SFXType)
{
    if (!UISoundDataAsset) return;
    USoundBase* Sound = UISoundDataAsset->GetUISFX(SFXType);
    if (Sound)
    {
        UGameplayStatics::PlaySound2D(this, Sound);
    }
}

bool UAZSoundManagerSubsystem::TraceFootSurface(AActor* Actor, FVector& OutFootLocation, ESurfaceType& OutSurfaceType) const
{
    OutSurfaceType = ESurfaceType::Metal;

    if (!Actor) return false;

    UCapsuleComponent* Capsule = Actor->FindComponentByClass<UCapsuleComponent>();
    if (!Capsule) return false;

    const FVector ActorLocation = Actor->GetActorLocation();
    const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

    const FVector CapsuleBottomLocation =
        ActorLocation - FVector(0.f, 0.f, CapsuleHalfHeight);

    OutFootLocation = CapsuleBottomLocation;

    FHitResult HitResult;

    const FVector TraceStart = CapsuleBottomLocation;
    const FVector TraceEnd = TraceStart - FVector(0.f, 0.f, 100.f);

    FCollisionQueryParams Params;
    Params.bReturnPhysicalMaterial = true;
    Params.AddIgnoredActor(Actor);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_GameTraceChannel7,
        Params
    );

    if (!bHit || !HitResult.PhysMaterial.IsValid())
    {
        return false;
    }

    const EPhysicalSurface PhysicalSurface =
        UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

    switch (PhysicalSurface)
    {
    case SurfaceType1:
        OutSurfaceType = ESurfaceType::Metal;
        break;

    case SurfaceType2:
        OutSurfaceType = ESurfaceType::Concrete;
        break;

    default:
        break;
    }

    return true;
}
