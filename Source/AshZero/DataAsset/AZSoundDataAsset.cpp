// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/AZSoundDataAsset.h"

TObjectPtr<USoundBase> UAZSoundDataAsset::GetFootStepSound(EActorType ActorType, ESurfaceType SurfaceType)
{
    FSFXCollection* Collection = SFXList.Find(ActorType);
    if (!Collection) return nullptr;
    USoundBase* Footsteps = Collection->FootStepMap.FindRef(SurfaceType);
    if (!Footsteps) return nullptr;

    return Footsteps;
}

TObjectPtr<USoundBase> UAZSoundDataAsset::GetSound(EActorType ActorType, ESFXType SFXType)
{
    FSFXCollection* Collection = SFXList.Find(ActorType);
    if (!Collection)
    {
        return nullptr;
    }

    return Collection->SFXMap.FindRef(SFXType);
}
