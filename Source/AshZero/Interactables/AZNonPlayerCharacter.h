// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/AZInteractable.h"
#include "Util/AZDefine.h"
#include "AZNonPlayerCharacter.generated.h"

class UCapsuleComponent;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteraction, AActor*, Actor);

UCLASS()
class ASHZERO_API AAZNonPlayerCharacter : public AActor, public IAZInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZNonPlayerCharacter();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCapsuleComponent> Collision;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly)
	FEnableButtons EnableButtons;

	UPROPERTY(EditDefaultsOnly, Category = "GameplayTag", meta = (Categories = "NPC"))
	FGameplayTag NameTag;

	FOnInteraction OnInteraction;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

};
