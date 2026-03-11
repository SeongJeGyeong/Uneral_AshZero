// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AZPlaySoundNotify.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/AZSoundDataAsset.h"
#include "Enemy/AZEnemyBase.h"
#include "Enemy/AZBossBase.h"

void UAZPlaySoundNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (SFXType == ESFXType::None) return;

	if (!MeshComp) return;

	APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner());
	if (!OwnerPawn) return;

	UWorld* World = OwnerPawn->GetWorld();
	if (!World) return;

#if WITH_EDITOR
	if (!World->IsGameWorld())
	{
		TSoftObjectPtr<UAZSoundDataAsset> SoundData =
			TSoftObjectPtr<UAZSoundDataAsset>(FSoftObjectPath(TEXT("/Game/Blueprints/Data/DataAssets/DA_SoundData.DA_SoundData")));

		if (SoundData.IsPending())
		{
			EActorType ActorType = EActorType::A_Player;

			if (OwnerPawn->IsA(AAZBossBase::StaticClass()))
			{
				ActorType = EActorType::A_Boss;
			}
			else if (OwnerPawn->IsA(AAZEnemyBase::StaticClass()))
			{
				ActorType = EActorType::A_Enemy;
			}

			FSFXCollection* SFXCollection = SoundData.LoadSynchronous()->SFXList.Find(ActorType);
			if (!SFXCollection) return;
			if (SFXType == ESFXType::FootStep)
			{
				USoundBase* Footstep =  SFXCollection->FootStepMap.FindRef(ESurfaceType::Concrete);
				if(Footstep) UGameplayStatics::PlaySound2D(World, Footstep);
			}
			else
			{
				USoundBase* SFX = SFXCollection->SFXMap.FindRef(SFXType);
				if (SFX) UGameplayStatics::PlaySound2D(World, SFX);
			}
		}

		return;
	}
#endif


	if (!OwnerPawn->IsLocallyControlled())
		return;

	// 예시: 게임 인스턴스 기반 사운드 매니저
	if (UGameInstance* GI = OwnerPawn->GetGameInstance())
	{
		UAZSoundManagerSubsystem* SoundManager = GI->GetSubsystem<UAZSoundManagerSubsystem>();
		if (!SoundManager) return;

		if (SFXType == ESFXType::FootStep)
		{
			SoundManager->PlayFootstepSFX(OwnerPawn);
		}
		else
		{
			SoundManager->PlaySFX(OwnerPawn, SFXType);
		}
	}

}