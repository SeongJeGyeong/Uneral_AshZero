// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Util/AZDefine.h"
#include "AZSoundManagerSubsystem.generated.h"

class UAZSoundDataAsset;
class UAudioComponent;
class UAZUISoundDataAsset;

UCLASS(Blueprintable)
class ASHZERO_API UAZSoundManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	UFUNCTION(BlueprintCallable)
	void PlayBGM(EBGMType Type);
	UFUNCTION(BlueprintCallable)
	void StopBGM();

	void PlayFootstepSFX(AActor* Actor);
	void PlaySFX(AActor* Actor, ESFXType SFXType);
	UFUNCTION(BlueprintCallable)
	void PlayUISFX(EUISFXType SFXType);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Data")
	TObjectPtr<UAZSoundDataAsset> SoundDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Data")
	TObjectPtr<UAZUISoundDataAsset> UISoundDataAsset;
private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> MusicAudioComponent;

	bool TraceFootSurface(AActor* Actor, FVector& OutFootLocation, ESurfaceType& OutSurfaceType) const;

};
