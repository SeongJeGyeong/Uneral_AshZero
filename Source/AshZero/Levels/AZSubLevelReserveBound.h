// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AZSubLevelReserveBound.generated.h"

class UBoxComponent;

UCLASS()
class ASHZERO_API AAZSubLevelReserveBound : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZSubLevelReserveBound();
	void InitializeReserver(FVector BoxExtent);

	UPROPERTY(VisibleAnywhere, Category = "Room Name")
	FString RoomName;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> ReserveBound;

};
