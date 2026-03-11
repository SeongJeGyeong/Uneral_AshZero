// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Util/AZDefine.h"
#include "AZInteractionUIComponent.generated.h"

class UAZInteractionWidget;
class UPostProcessComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZInteractionUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZInteractionUIComponent();

	UPROPERTY()
	TObjectPtr<UAZInteractionWidget> InteractionWidget;

	UPROPERTY()
	TObjectPtr<UPostProcessComponent> PostProcessComponent;

	UPROPERTY(EditAnywhere, Category = "AZ|UI")
	TSubclassOf<UAZInteractionWidget> InteractionWidgetClass;

	UPROPERTY()
	FMargin WidgetScreenMargin;

	float ScreenRadiusPercent = 0.5f;

	void SetVisibleState(ESlateVisibility Visibility);

	void TargetHighlighted(AActor* Target, bool bIsHighlighted);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool IsWorldValid() const;

	void ConstructMaterial();
	void ConstructUI();
	void DestroyUI();
	void UpdateInteractable();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<APlayerController> Owner;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> MaterialInterface;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> OutlineMaterial;

	UPROPERTY()
	TObjectPtr<AActor> FocusTarget;

	double CurrentTargetDot = 0.f;
	bool bIsVisible = true;

private:
	FLinearColor SwitchIntToColor(uint32 Color);

	UFUNCTION()
	void OnPawnChanged(APawn* OldPawn, APawn* NewPawn);
};
