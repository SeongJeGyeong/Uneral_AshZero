// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/AZInteractable.h"
#include "Components/TimelineComponent.h"
#include "AZChest.generated.h"

class UAZLootBoxComponent;
class UAZItemBase;
class UAZItemWidget;
class UMaterialInstanceDynamic;
UCLASS()
class ASHZERO_API AAZChest : public AActor, public IAZInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZChest();
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void SetIsItemGenerated_Multicast(bool bIsItemGenerate);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	UFUNCTION(Server, Reliable)
	void SetIsItemGenerated_Server(bool bIsItemGenerate);

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAZLootBoxComponent> LootBoxComponent;

	void InitBagItems();

	UPROPERTY()
	bool bIsItemsGenerated = false;

	void GenerateRewardBox(const TArray<FAZQuestReward>& Rewards);

	void RegisterInteractor(AActor* User);
	void UnregisterInteractor(AActor* User);
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Dissolve")
	UCurveFloat* DissolveCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float DissolveDuration = 3.0f;

	UPROPERTY(Transient)
	TSet<AActor*> CurrentInteractors;
private:
	FTimeline DissolveTimeline;
	TArray<UMaterialInstanceDynamic*> CachedMIDs;
	bool bIsDisappearing = false;

	UFUNCTION()
	void HandleDissolveProgress(float Value);

	UFUNCTION()
	void HandleDissolveFinished();

	UFUNCTION(NetMulticast, Reliable)
	void StartDestroyEffect_Multicast();

	void CheckAndStartDissolve();

	bool IsLootBoxEmpty() const;
};
