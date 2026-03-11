// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AZRoomExitNode.generated.h"

class AAZDoor;

UCLASS()
class ASHZERO_API UAZRoomExitNode : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	int32 NodeID = 0;

	UPROPERTY()
	int32 OwnerIndex = INDEX_NONE;
	UPROPERTY()
	FTransform ExitTransform = FTransform::Identity;

	UPROPERTY()
	TObjectPtr<AAZDoor> AttachedDoor = nullptr;
};
