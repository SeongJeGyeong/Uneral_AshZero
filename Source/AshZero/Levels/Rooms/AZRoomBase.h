// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AZRoomBase.generated.h"

UCLASS()
class ASHZERO_API AAZRoomBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZRoomBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsSeparateOverlappingRooms();
	void HighlightRoom();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* Cube;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterial* OrangeMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "ToMoveWithCollision"))
	bool bToMove = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "ToMoveWithCollision"))
	bool bToSeparate = false;

	// the room scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "ScaleOfRoom"))
	int32 Scale = 0;

	// Is Main Room
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "IsMain"))
	bool bIsMain = false;
};
