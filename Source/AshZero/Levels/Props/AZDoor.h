// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AZDoor.generated.h"

class UBoxComponent;
class AAZBaseRoom;

UCLASS()
class ASHZERO_API AAZDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZDoor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> Door;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> DoorFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor NormalColor = FLinearColor(0.5f, 0.07f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor OpenColor = FLinearColor(0.f, 0.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ClosedColor = FLinearColor(1.f, 0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NormalEmissive = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RadiateEmissive = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDoorOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMoving = false;

	UPROPERTY()
	TObjectPtr<AAZBaseRoom> OwnerRoom;

public:	
	void SetDoorTrigger(bool bActivate);

	UFUNCTION(BlueprintCallable)
	void SensingDoor(bool bSense);

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void CheckDoorState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorSensedEffect(bool bIsOpening);

	UFUNCTION()
	void OnDoorActive(bool bDoorOpen);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 OverlappedCount = 0;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DoorMID;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DoorFrameMID;

	FTimerHandle OpenDelayTimerHandle;
	FTimerHandle CloseDelayTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Door Settings", meta = (AllowPrivateAccess = "true"))
	float OpenDelayTime = 0.5f;

	void ExecuteOpen();
	void ExecuteClose();
};
