// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AZPlayerBaseComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZPlayerBaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAZPlayerBaseComponent();

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

public:	
	// 사용자 입력 맵핑 처리 함수
	virtual void SetupInputBinding(class UEnhancedInputComponent* PlayerInput) {};

	// 컴포넌트 소유 액터
	UPROPERTY()
	class AAZPlayerCharacter* OwnerCharacter;

	UPROPERTY()
	class UCharacterMovementComponent* CharacterMoveComp;
};
