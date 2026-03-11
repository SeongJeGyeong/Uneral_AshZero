// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AZPlayerBaseComponent.h"
#include "AZInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableFound, AActor*, InteractableActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableLost);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZInteractionComponent : public UAZPlayerBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZInteractionComponent();
	virtual void SetupInputBinding(class UEnhancedInputComponent* PlayerInput) override;
	//플레이어가 F키 눌렀을 때 호출할 함수
	void PrimaryInteract();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void FindInteractable();

	UPROPERTY(EditAnywhere, Category = "AZ|Interaction")
	float TraceDistance = 100.0f;

	UPROPERTY(EditAnywhere, Category = "AZ|Interaction")
	float TraceRadius = 50.0f;

public:	
	// UI 업데이트용 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnInteractableFound OnInteractableFound;

	UPROPERTY(BlueprintAssignable)
	FOnInteractableLost OnInteractableLost;

	TObjectPtr<AActor> GetFocusedActor() const { return FocusedActorCached; }

private:
	//현재 상호작용 가능한 액터
	UPROPERTY()
	TObjectPtr<UObject> FocusedObject;

	UPROPERTY()
	TObjectPtr<AActor> FocusedActorCached;

};
