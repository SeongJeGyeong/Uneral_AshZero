// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AZPlayerBaseComponent.h"
#include "Util/AZDefine.h"
#include "AZPlayerFireComponent.generated.h"

/**
 * 플레이어 발사 관련 컴포넌트
 * 무기 발사, 재장전, 네트워크 동기화
 */

class AAZWeapon;
class UAZCrosshairWidget;
class UCameraComponent;
class USpringArmComponent;
class UAZItemBase;
class AAZThrowProjectile;
struct FAZThrowablesItemDataTable;

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponChangedDelegate, AAZWeapon*, CurrentWeapon, AAZWeapon*, SubWeapon, int32, CurrentSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedDelegate, int32, CurrentAmmo, int32, MaxAmmo);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ASHZERO_API UAZPlayerFireComponent : public UAZPlayerBaseComponent
{
	GENERATED_BODY()

public:
	UAZPlayerFireComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void SetupInputBinding(class UEnhancedInputComponent* PlayerInput) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==================== 무기 슬롯 ====================
	// 장착된 무기 (2슬롯)
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapons, BlueprintReadOnly, Category = "AZ|Weapon")
	TArray<AAZWeapon*> EquippedWeapons;

	// 현재 선택된 슬롯 (0 또는 1)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AZ|Weapon")
	int32 CurrentSlotIndex = 0;

	// 현재 손에 들고 있는 무기
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "AZ|Weapon")
	AAZWeapon* CurrentWeapon;

	// 시작 무기 클래스 (기본 권총)
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Weapon")
	TSubclassOf<AAZWeapon> StartWeaponClass;

	// 서브 무기 클래스
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Weapon")
	TSubclassOf<AAZWeapon> SubWeaponClass;

	// ==================== 델리게이트 ====================
	UPROPERTY(BlueprintAssignable, Category = "AZ|Weapon")
	FOnWeaponChangedDelegate OnWeaponChanged;

	UPROPERTY(BlueprintAssignable, Category = "AZ|Weapon")
	FOnAmmoChangedDelegate OnAmmoChanged;

	// ==================== 무기 교체 ====================
	UFUNCTION(BlueprintCallable, Category = "AZ|Weapon")
	void SwitchWeapon();

	UFUNCTION(BlueprintCallable, Category = "AZ|Weapon")
	void SwitchToSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "AZ|Weapon")
	void EquipWeaponToSlot(TSubclassOf<AAZWeapon> WeaponClass, int32 SlotIndex, int32 WeaponLevel);
	
	UFUNCTION(BlueprintCallable, Category = "AZ|Weapon")
	void UnEquipWeaponFromSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "AZ|Weapon")
	AAZWeapon* GetSubWeapon() const;

	// 무기 스왑 중 여부
	UPROPERTY(BlueprintReadOnly, Category = "AZ|Weapon")
	bool bIsSwapping = false;

	UFUNCTION(BlueprintCallable, Category = "AZ|Weapon")
	void OnWeaponSwapEnd();

	// ==================== 재장전 ====================
	// AnimNotify에서 호출 (AZFuncCallAnimNotify 사용)
	UFUNCTION(BlueprintCallable, Category = "AZ|Weapon")
	void OnReloadComplete();

	// 재장전 캔슬 (구르기 등에서 호출)
	UFUNCTION(BlueprintCallable, Category = "AZ|Weapon")
	void CancelReload();

	// 재장전 중 여부
	UPROPERTY(BlueprintReadOnly, Category = "AZ|Weapon")
	bool bIsReloading = false;


	// ==================== 조준 ====================
	// 조준 강제 해제 (구르기 등에서 호출)
	UFUNCTION(BlueprintCallable, Category = "AZ|Aiming")
	void ForceEndAim();

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Aiming")
	bool bIsAiming = false;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Aiming")
	float AimingSpeedMultiplier = 0.9f;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Aiming")
	float DefaultSpread = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Aiming")
	float AimingSpread = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Aiming")
	float DefaultFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Aiming")
	float AimingFOV = 45.f;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Aiming")
	float AimZoomSpeed = 10.f;

	// ==================== 반동 ====================
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Recoil")
	float RecoilPitch = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Recoil")
	float RecoilYaw = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Recoil")
	float RecoilRecoverySpeed = 5.f;

	// ==================== 제자리 회전 ====================
	UPROPERTY(EditAnywhere, Category = "AZ|TurnInPlace")
	float TurnInPlaceAngle = 90.f;

	UPROPERTY(EditAnywhere, Category = "AZ|TurnInPlace")
	float TurnInPlaceTime = 5.f;

	UPROPERTY(EditAnywhere, Category = "AZ|TurnInPlace")
	float TurnInPlaceEndAngle = 3.f;

	ETurnInPlace TurnInPlaceType = ETurnInPlace::ETIP_NotTurning;
	float AimOffset_InterpYaw = 0;

	// 에임 오프셋 (애니메이션용)
	float AimOffset_Yaw = 0;
	float AimOffset_Pitch = 0;
	float AimOffset_StartYaw = 0;

	// ==================== UI ====================
	void InitializeUI();

	UPROPERTY(EditDefaultsOnly, Category = "AZ|UI")
	TSubclassOf<UAZCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY()
	UAZCrosshairWidget* CrosshairWidget;

	// ==================== 카메라 ====================
	UPROPERTY()
	UCameraComponent* CamComp;

	UPROPERTY()
	USpringArmComponent* SpringArmComp;

	void TurnInPlace(float DeltaTime);
	float GetRemoteControlYaw();

	// 사격 중단 (외부에서 호출용 - 구르기 등)
	void InputFireEnd(const struct FInputActionValue& InputValue);

	UFUNCTION(Server, Reliable)
	void StartThrowGrenade_Server(int32 ItemID);

	UFUNCTION(NetMulticast, Reliable)
	void StartThrowGrenade_Multicast(int32 ItemID);
 
	UFUNCTION(BlueprintCallable, Category = "AZ|Weapon")
	void SpawnGrenadeInHand();

	UFUNCTION(BlueprintCallable, Category = "AZ|Weapon")
	void ExecuteThrowGrenade();

	UFUNCTION(Server, Reliable)
	void SpawnGrenadeProjectile_Server();

	UFUNCTION(Server, Reliable)
	void ExecuteThrowGrenadeProjectile_Server();

	// ==================== 자유 시점 모드 ====================
	public:
		UPROPERTY(BlueprintReadOnly, Category = "AZ|State")
		bool bIsFreeLookMode = false;

		void SetFreeLookMode(bool bEnable);

protected:
	// ==================== 입력 처리 ====================
	void InputFireStart(const struct FInputActionValue& InputValue);
	void InputAimStart(const FInputActionValue& InputValue);
	void InputAimEnd(const FInputActionValue& InputValue);
	void InputReload(const struct FInputActionValue& InputValue);
	void InputSwitchWeapon(const FInputActionValue& InputValue);

	// ==================== 발사 처리 ====================
	void Fire();
	void LocalFire(FVector StartPos, FVector Direction);
	void UpdateAimZoom(float DeltaTime);

	void ApplyRecoil();
	void RecoverRecoil(float DeltaTime);
	FVector ApplySpread(FVector Direction, float SpreadAngle);

	void BroadcastAmmoChange();

	// ==================== 네트워크 ====================
	UFUNCTION(Server, Reliable)
	void Server_Fire(FVector_NetQuantize StartPos, FVector_NetQuantizeNormal Direction);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(FVector_NetQuantize StartPos, FVector_NetQuantizeNormal Direction);

	UFUNCTION(Server, Reliable)
	void Server_Reload();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Reload();

	UFUNCTION(Server, Reliable)
	void Server_SwitchToSlot(int32 SlotIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SwitchToSlot(int32 SlotIndex);
	// ==================== OnRep ====================
	UFUNCTION()
	void OnRep_CurrentWeapon();

	UFUNCTION()
	void OnRep_EquippedWeapons();

	// ==================== 내부 함수 ====================
	void Internal_SwitchToSlot(int32 SlotIndex);

	// 타이머 핸들
	FTimerHandle FireTimerHandle;

	// 반동 오프셋
	FVector2D CurrentRecoilOffset = FVector2D::ZeroVector;
	FVector2D TargetRecoilOffset = FVector2D::ZeroVector;

private:
	bool bIsFiring = false;

	void UpdateAimOffset(float DeltaTime);

	UPROPERTY(Replicated)
	int32 ThrowableItemID = 0;

	UPROPERTY(EditDefaultsOnly, Category = "AZ|Class")
	TSubclassOf<AAZThrowProjectile> GrenadeClass;

	UPROPERTY();
	AAZThrowProjectile* Grenade;
};