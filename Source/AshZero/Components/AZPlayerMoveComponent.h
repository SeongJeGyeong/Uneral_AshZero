// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AZPlayerBaseComponent.h"
#include "AZPlayerMoveComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChangedDelegate, float, CurrentStamina, float, MaxStamina);




UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ASHZERO_API UAZPlayerMoveComponent : public UAZPlayerBaseComponent
{
	GENERATED_BODY()

public:
	UAZPlayerMoveComponent();
	virtual void BeginPlay() override;
	virtual void SetupInputBinding(class UEnhancedInputComponent* PlayerInput) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==================== 이동 속도 ====================
	UPROPERTY(EditAnywhere, Category = "AZ|Movement")
	float BaseWalkSpeed = 200.f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_UpdateSpeed, Category = "AZ|Movement")
	float WalkSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = "AZ|Movement")
	float BaseRunSpeed = 600.f;

	UPROPERTY(EditAnywhere, Replicated, Category = "AZ|Movement")
	float RunSpeed = 600.f;

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Movement")
	bool bIsRun = false;

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Movement")
	float SpeedMultiplier = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "AZ|Movement")
	void SetSpeedMultiplier(float Multiplier);

	UFUNCTION()
	void OnRep_UpdateSpeed();

	// ==================== 스테미나 ====================
	UPROPERTY(EditAnywhere, Category = "AZ|Stamina")
	float BaseStamina = 100.f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_MaxStamina, Category = "AZ|Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "AZ|Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, Category = "AZ|Stamina")
	float StaminaDrainRate = 20.f;

	UPROPERTY(EditAnywhere, Category = "AZ|Stamina")
	float StaminaRegenRate = 15.f;

	UPROPERTY(EditAnywhere, Category = "AZ|Stamina")
	float StaminaRegenDelay = 1.0f;

	UPROPERTY(BlueprintAssignable, Category = "AZ|Stamina")
	FOnStaminaChangedDelegate OnStaminaChanged;

	UFUNCTION(BlueprintPure, Category = "AZ|Stamina")
	float GetStaminaPercent() const { return MaxStamina > 0.f ? CurrentStamina / MaxStamina : 0.f; }

	UFUNCTION()
	void OnRep_Stamina();

	UFUNCTION()
	void OnRep_MaxStamina();

	// ==================== 구르기 ====================
	UPROPERTY(EditAnywhere, Category = "AZ|Roll")
	float RollStaminaCost = 25.f;

	UPROPERTY(EditAnywhere, Category = "AZ|Roll")
	float RollCooldown = 0.8f;

	UPROPERTY(EditAnywhere, Category = "AZ|Roll")
	float RollDistance = 400.f;

	UPROPERTY(EditAnywhere, Category = "AZ|Roll")
	float RollDuration = 0.65f;

	UPROPERTY(EditAnywhere, Category = "AZ|Roll")
	float InvincibleTime = 0.3f;

	UPROPERTY(EditAnywhere, Category = "AZ|Roll")
	float RollEaseExponent = 2.f;  // 감속 강도 (1.5 약함 ~ 3.0 강함)

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Roll")
	bool bIsRolling = false;

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Roll")
	bool bCanRoll = true;

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Roll")
	bool bIsRecoveringFromRoll = false;

	float RecoveryElapsed = 0.f;	// 구르기 후 회복 시간 측정

	UPROPERTY(EditDefaultsOnly, Category = "AZ|VFX")
	class UNiagaraSystem* RollVFX;

	UPROPERTY(EditAnywhere, Category = "AZ|VFX")
	FName RollVFXSocket = FName("spine_03"); 

	UPROPERTY(EditAnywhere, Category = "AZ|VFX")
	FVector RollVFXOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "AZ|Roll")
	float RotateDuration = 0.1f;

	UPROPERTY()
	class UNiagaraComponent* RollVFXComponent;

	// ==================== 입력 함수 ====================
	void Look(const struct FInputActionValue& InputValue);
	void Move(const struct FInputActionValue& InputValue);
	void SprintStart(const struct FInputActionValue& InputValue);
	void SprintEnd(const struct FInputActionValue& InputValue);
	void Roll(const struct FInputActionValue& InputValue);
	void Interact(const struct FInputActionValue& InputValue);

	// ==================== 내부 처리 ====================
	void HandleRolling();
	void EndRoll();
	void StartInvincible(float Duration);

	// ==================== 네트워크 ====================
	UFUNCTION(Server, Reliable)
	void Server_Roll(FVector RollDirection);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Roll(FVector RollDirection);

	UFUNCTION(Server, Reliable)
	void Server_SprintStart();

	UFUNCTION(Server, Reliable)
	void Server_SprintEnd();

	UFUNCTION(Server, Reliable)
	void Server_Interact();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SprintStart();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SprintEnd();

	UFUNCTION(Server, Reliable)
	void UpdateSpeed_Server(float PercentBonus, float PercentPenalty);

	// ==================== 기타 ====================
	UPROPERTY()
	AActor* InteractableActor;

	void ResetMoveInput() { CurrentMoveInput = FVector2D::ZeroVector; }

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Movement")
	bool bIsFreeLookMode = false;  // true = 마을, false = 인게임

	UFUNCTION(BlueprintCallable, Category = "AZ|Movement")
	void SetFreeLookMode(bool bEnable);

private:
	float TimeSinceStaminaUse = 0.f;
	void UpdateStamina(float DeltaTime);
	void ResetRollCooldown();

	FTimerHandle RollCooldownHandle;
	FTimerHandle InvincibleTimerHandle;

protected:
	FVector2D CurrentMoveInput;
	FVector RollStartLocation;
	FVector RollTargetLocation;
	float RollElapsed = 0.f;
	float TurnElapsed = 0.f;
	FTimerHandle RollTimerHandle;

	FRotator RollStartRotation;
	FRotator RollTargetRotation;

	bool bIsRotatingAfterRoll = false;
};