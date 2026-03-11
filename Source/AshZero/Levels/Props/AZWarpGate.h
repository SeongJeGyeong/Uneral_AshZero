// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AZWarpGate.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class USceneCaptureComponentCube;
class UNiagaraSystem;
class UNiagaraComponent;
class AAZPlayerCharacter;

UCLASS()
class ASHZERO_API AAZWarpGate : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> GateMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneCaptureComponentCube> RenderCaptureCube;

	UPROPERTY(EditAnywhere, Category = "VFX")
	TObjectPtr<UNiagaraSystem> TeleportNiagara;

	int32 OverlappedCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Warp")
	float WarpDuration = 5.f;

	// 서버 타이머
	FTimerHandle WarpTimerHandle;

	// 서버 기준 누적 시간
	float WarpElapsedTime = 0.f;

	// 현재 조건 만족 여부
	bool bWarpInProgress = false;

public:	
	// Sets default values for this actor's properties
	AAZWarpGate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void StartWarpProgress();
	void UpdateWarpProgress();
	void ResetWarpProgress();
	void CompleteWarp();

	UFUNCTION(NetMulticast, Reliable)
	void CompleteWarp_Multicast();

	UFUNCTION(NetMulticast, Reliable)
	void ShowWarpUI_Multicast(bool bShow);

	UFUNCTION(NetMulticast, Reliable)
	void UpdateWarpProgress_Multicast(float Progress);

	UFUNCTION(NetMulticast, Reliable)
	void CreateWarpWidget_Multicast();

	UFUNCTION(Server, Reliable)
	void EvaluateWarpProgress_Server();

	UFUNCTION()
	void WarpFinished(UNiagaraComponent* FinishedComponent);

	UPROPERTY()
	TArray<TObjectPtr<AAZPlayerCharacter>> OverlappedCharacters;
};
