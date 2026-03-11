// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MyCharacter.generated.h"

UCLASS()
class ASHZERO_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UInputMappingContext> IMC;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UInputAction> IA_Jump;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	//void Jump(const FInputActionValue& Value);
};
