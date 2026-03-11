// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Util/AZDefine.h"
#include "AZItemUsageComponent.generated.h"

class AAZPlayerController;
class UAZItemBase;
class UAZEquipmentComponent;
class UNiagaraSystem;
struct FAZSuppliesItemDataTableRow;
struct FAZThrowablesItemDataTable;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZItemUsageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZItemUsageComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FTimerHandle ItemUseTimerHandle;

	void ProcessItemUsage(UAZItemBase* ItemToUse, ESlotIndex SlotIndex);
	void ApplySuppliesEffect(UAZItemBase* Item, const FAZSuppliesItemDataTableRow* Data);
	void ApplyThrowablesEffect(UAZItemBase* Item, const FAZThrowablesItemDataTable* Data);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetupInputBinding(class UEnhancedInputComponent* PlayerInputComponent);

	void OnQuickSlot_1(const struct FInputActionValue& Value);
	void OnQuickSlot_2(const struct FInputActionValue& Value);
	void OnQuickSlot_3(const struct FInputActionValue& Value);
	void OnQuickSlot_4(const struct FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Visual", meta = (DisplayPriority = "30"))
	TObjectPtr<UNiagaraSystem> HealingVFX;

	bool IsUsingItem();

	UFUNCTION(Server, Reliable)
	void RequestUseItem_Server(UAZItemBase* ItemToUse, ESlotIndex SlotIndex);
private:
	UPROPERTY()
	TObjectPtr<UAZEquipmentComponent> EquipmentComponent;

	
};
