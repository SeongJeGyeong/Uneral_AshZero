// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Util/AZDefine.h"
#include "AZSoundDataAsset.generated.h"

class USoundBase;

USTRUCT(BlueprintType)
struct FSFXCollection
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<ESurfaceType, TObjectPtr<USoundBase>> FootStepMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<ESFXType, TObjectPtr<USoundBase>> SFXMap;
};

UCLASS()
class ASHZERO_API UAZSoundDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM")
	TMap<EBGMType, TObjectPtr<USoundBase>> BGMList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TMap<EActorType, FSFXCollection> SFXList;


	TObjectPtr<USoundBase> GetFootStepSound(EActorType ActorType, ESurfaceType SurfaceType);
	TObjectPtr<USoundBase> GetSound(EActorType ActorType, ESFXType SFXType);
};
