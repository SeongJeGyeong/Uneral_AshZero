// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AZPlayerFireComponent.h"
#include "Components/AZPlayerMoveComponent.h"
#include "Animation/AZPlayerAnimInstance.h"
#include "Character/AZPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Util/AZDefine.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Weapon/AZWeapon.h"
#include "AZDataAsset.h"
#include "AZGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "UI/HUD/AZCrosshairWidget.h"
#include "AshZero.h"
#include "System/Player/AZPlayerController.h"
#include "Components/AZItemUsageComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Item/AZThrowProjectile.h"
#include "System/AZDataManagerSubsystem.h"
#include "DataTable/AZThrowablesItemDataTable.h"
#include "Components/AZEquipmentComponent.h"

UAZPlayerFireComponent::UAZPlayerFireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	EquippedWeapons.SetNum(2);
}

void UAZPlayerFireComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<AAZPlayerCharacter>(GetOwner());
	}

	if (OwnerCharacter)
	{
		CamComp = OwnerCharacter->CamComp;
		SpringArmComp = OwnerCharacter->SpringArmComp;

		if (CamComp)
		{
			DefaultFOV = CamComp->FieldOfView;
		}
	}
}

void UAZPlayerFireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CamComp && OwnerCharacter)
	{
		CamComp = OwnerCharacter->CamComp;
		SpringArmComp = OwnerCharacter->SpringArmComp;

		if (CamComp)
		{
			DefaultFOV = CamComp->FieldOfView;
			UE_LOG(LogTemp, Warning, TEXT("[FireComp] CamComp cached in Tick!"));
		}
	}

	if (OwnerCharacter)
	{
		UpdateAimOffset(DeltaTime);
		RecoverRecoil(DeltaTime);
		UpdateAimZoom(DeltaTime);
	}
}

void UAZPlayerFireComponent::SetupInputBinding(UEnhancedInputComponent* PlayerInput)
{
	if (!PlayerInput || !OwnerCharacter || !OwnerCharacter->InputDataAsset)
	{
		return;
	}

	UAZDataAsset* InputData = OwnerCharacter->InputDataAsset;
	if (InputData == nullptr) return;

	PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Fire), ETriggerEvent::Started, this, &UAZPlayerFireComponent::InputFireStart);
	PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Fire), ETriggerEvent::Completed, this, &UAZPlayerFireComponent::InputFireEnd);
	PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Reload), ETriggerEvent::Started, this, &UAZPlayerFireComponent::InputReload);
	PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Aim), ETriggerEvent::Started, this, &UAZPlayerFireComponent::InputAimStart);
	PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_Aim), ETriggerEvent::Completed, this, &UAZPlayerFireComponent::InputAimEnd);
	PlayerInput->BindAction(InputData->FindInputActionByTag(AZGameplayTags::Input_Action_SwitchWeapon), ETriggerEvent::Triggered, this, &UAZPlayerFireComponent::InputSwitchWeapon);
}

void UAZPlayerFireComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UAZPlayerFireComponent, CurrentWeapon, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(UAZPlayerFireComponent, EquippedWeapons);
	DOREPLIFETIME(UAZPlayerFireComponent, CurrentSlotIndex);
	DOREPLIFETIME(UAZPlayerFireComponent, ThrowableItemID);
}

void UAZPlayerFireComponent::InitializeUI()
{
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled()) return;

	CamComp = OwnerCharacter->CamComp;
	SpringArmComp = OwnerCharacter->SpringArmComp;

	if (CamComp)
	{
		DefaultFOV = CamComp->FieldOfView;
		UE_LOG(LogTemp, Warning, TEXT("[InitializeUI] CamComp cached successfully!"));
	}

	if (CrosshairWidgetClass && !CrosshairWidget)
	{
		CrosshairWidget = CreateWidget<UAZCrosshairWidget>(GetWorld(), CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport();
			if (bIsFreeLookMode)
			{
				CrosshairWidget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	AAZWeapon* SubWeapon = GetSubWeapon();
	OnWeaponChanged.Broadcast(CurrentWeapon, SubWeapon, CurrentSlotIndex);
	BroadcastAmmoChange();
}

void UAZPlayerFireComponent::SwitchToSlot(int32 SlotIndex)
{
	if (!EquippedWeapons.IsValidIndex(SlotIndex)) return;
	if (EquippedWeapons[SlotIndex] == nullptr) return;
	if (SlotIndex == CurrentSlotIndex) return;

	if (GetOwnerRole() == ROLE_Authority)
	{
		Multicast_SwitchToSlot(SlotIndex);
	}
	else
	{
		Server_SwitchToSlot(SlotIndex);
	}
}

void UAZPlayerFireComponent::Server_SwitchToSlot_Implementation(int32 SlotIndex)
{
	Multicast_SwitchToSlot(SlotIndex);
}

void UAZPlayerFireComponent::Multicast_SwitchToSlot_Implementation(int32 SlotIndex)
{
	Internal_SwitchToSlot(SlotIndex);
}

void UAZPlayerFireComponent::Internal_SwitchToSlot(int32 SlotIndex)
{
	if (!EquippedWeapons.IsValidIndex(SlotIndex)) return;

	AAZWeapon* NewWeapon = EquippedWeapons[SlotIndex];
	if (!NewWeapon) return;

	// 무기 교체 시 재장전 캔슬
	CancelReload();

	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(true);
		CurrentWeapon->SetActorEnableCollision(false);
	}

	CurrentSlotIndex = SlotIndex;
	CurrentWeapon = NewWeapon;

	CurrentWeapon->SetActorHiddenInGame(false);
	CurrentWeapon->SetActorEnableCollision(true);

	if (OwnerCharacter)
	{
		CurrentWeapon->Equip(OwnerCharacter);
		OwnerCharacter->CurrentWeapon = CurrentWeapon;
	}

	AAZWeapon* SubWeapon = GetSubWeapon();
	OnWeaponChanged.Broadcast(CurrentWeapon, SubWeapon, CurrentSlotIndex);
	BroadcastAmmoChange();

	UE_LOG(LogTemp, Warning, TEXT("[FireComp] Switched to slot %d, Weapon: %s"),
		SlotIndex, *CurrentWeapon->GetName());
}

void UAZPlayerFireComponent::EquipWeaponToSlot(TSubclassOf<AAZWeapon> WeaponClass, int32 SlotIndex, int32 WeaponLevel)
{
	if (!WeaponClass || !EquippedWeapons.IsValidIndex(SlotIndex)) return;
	if (GetOwnerRole() != ROLE_Authority) return;

	if (EquippedWeapons[SlotIndex])
	{
		EquippedWeapons[SlotIndex]->Destroy();
		EquippedWeapons[SlotIndex] = nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAZWeapon* NewWeapon = GetWorld()->SpawnActor<AAZWeapon>(WeaponClass, SpawnParams);
	if (NewWeapon)
	{
		EquippedWeapons[SlotIndex] = NewWeapon;
		NewWeapon->SetOwner(OwnerCharacter);

		if (SlotIndex == CurrentSlotIndex)
		{
			CurrentWeapon = NewWeapon;
			NewWeapon->Equip(OwnerCharacter);
			NewWeapon->SetLevel(WeaponLevel);
			if (OwnerCharacter)
			{
				OwnerCharacter->CurrentWeapon = CurrentWeapon;
			}
			Multicast_Reload();
		}
		else
		{
			NewWeapon->SetActorHiddenInGame(true);
			NewWeapon->SetActorEnableCollision(false);
		}

		AAZWeapon* SubWeapon = GetSubWeapon();
		OnWeaponChanged.Broadcast(CurrentWeapon, SubWeapon, CurrentSlotIndex);
		BroadcastAmmoChange();

		UE_LOG(LogTemp, Warning, TEXT("[FireComp] Equipped %s to slot %d"),
			*NewWeapon->GetName(), SlotIndex);
	}
}

void UAZPlayerFireComponent::UnEquipWeaponFromSlot(int32 SlotIndex)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (!EquippedWeapons.IsValidIndex(SlotIndex)) return;

	AAZWeapon* WeaponToRemove = EquippedWeapons[SlotIndex];
	if (WeaponToRemove == nullptr)	return;
	
	// 장착 해제하려는 무기가 현재 들고 있는 무기라면
	if (CurrentSlotIndex == SlotIndex)
	{
		int32 OtherSlotIndex = (SlotIndex + 1) % 2;
		if (EquippedWeapons.IsValidIndex(OtherSlotIndex) && EquippedWeapons[OtherSlotIndex] != nullptr)
		{
			SwitchToSlot(OtherSlotIndex);
		}
	}
	else
	{
		if (CurrentWeapon == WeaponToRemove)
		{
			CurrentWeapon = nullptr;
			if (OwnerCharacter)
			{
				OwnerCharacter->CurrentWeapon = nullptr;
			}
		}
	}
	
	WeaponToRemove->Destroy();
	EquippedWeapons[SlotIndex] = nullptr;

	AAZWeapon* SubWeapon = GetSubWeapon();
	if (OnWeaponChanged.IsBound())
	{
		OnWeaponChanged.Broadcast(CurrentWeapon, SubWeapon, CurrentSlotIndex);
	}
}

AAZWeapon* UAZPlayerFireComponent::GetSubWeapon() const
{
	int32 SubSlot = (CurrentSlotIndex + 1) % 2;
	if (EquippedWeapons.IsValidIndex(SubSlot))
	{
		return EquippedWeapons[SubSlot];
	}
	return nullptr;
}

void UAZPlayerFireComponent::OnWeaponSwapEnd()
{
	bIsSwapping = false;
}

// ==================== 재장전 시스템 ====================

void UAZPlayerFireComponent::OnReloadComplete()
{
	if (!bIsReloading) return;

	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
		BroadcastAmmoChange();
	}

	bIsReloading = false;
	UE_LOG(LogTemp, Log, TEXT("[FireComp] Reload Complete via AnimNotify"));
}

void UAZPlayerFireComponent::CancelReload()
{
	if (!bIsReloading) return;

	bIsReloading = false;
	UE_LOG(LogTemp, Log, TEXT("[FireComp] Reload Cancelled"));
}

void UAZPlayerFireComponent::SwitchWeapon()
{
	if (bIsSwapping || bIsReloading) return;

	if (OwnerCharacter && OwnerCharacter->bIsHitStunned) return;

	int32 NextSlot = (CurrentSlotIndex + 1) % 2;

	if (!EquippedWeapons.IsValidIndex(NextSlot) || !EquippedWeapons[NextSlot]) return;

	bIsSwapping = true;

	if (OwnerCharacter)
	{
		if (UAZPlayerAnimInstance* PlayerAnim = Cast<UAZPlayerAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
		{
			PlayerAnim->PlayWeaponSwapAnim();
		}
	}

	SwitchToSlot(NextSlot);
}

// ==================== 조준 시스템 ====================

void UAZPlayerFireComponent::ForceEndAim()
{
	if (!bIsAiming) return;

	bIsAiming = false;

	// 이동 속도 복구
	if (OwnerCharacter && OwnerCharacter->MoveComp)
	{
		OwnerCharacter->MoveComp->SetSpeedMultiplier(1.0f);
	}
	if (OwnerCharacter && OwnerCharacter->VignetteMID)
	{
		OwnerCharacter->VignetteMID->SetScalarParameterValue(TEXT("KernelSize"), 0.0f);
	}

	// 크로스헤어 UI 복구
	if (CrosshairWidget)
	{
		CrosshairWidget->SetCrosshairType(ECrosshairType::Default);
	}

	UE_LOG(LogTemp, Log, TEXT("[FireComp] Aim Force Ended"));
}

void UAZPlayerFireComponent::BroadcastAmmoChange()
{
	if (CurrentWeapon)
	{
		OnAmmoChanged.Broadcast(CurrentWeapon->CurrentAmmo, CurrentWeapon->MaxAmmo);
	}
}

void UAZPlayerFireComponent::InputSwitchWeapon(const FInputActionValue& InputValue)
{
	// 자유 시점 모드에서는 무기 교체 불가
	if (bIsFreeLookMode) return;
	// 구르기 중 무기 교체 불가
	if (OwnerCharacter && OwnerCharacter->MoveComp && OwnerCharacter->MoveComp->bIsRolling)
	{
		return;
	}

	if (AAZPlayerController* PC = Cast<AAZPlayerController>(OwnerCharacter->GetController()))
	{
		if (PC->ItemUsageComp && PC->ItemUsageComp->IsUsingItem())
		{
			return;
		}
	}
	PRINT_LOG(TEXT("Mouse Wheel Switch Weapon"));
	SwitchWeapon();
}

// ==================== OnRep 함수 ====================

void UAZPlayerFireComponent::OnRep_CurrentWeapon()
{
	if (CurrentWeapon && OwnerCharacter)
	{
		CurrentWeapon->Equip(OwnerCharacter);
		OwnerCharacter->CurrentWeapon = CurrentWeapon;

		AAZWeapon* SubWeapon = GetSubWeapon();
		OnWeaponChanged.Broadcast(CurrentWeapon, SubWeapon, CurrentSlotIndex);
		BroadcastAmmoChange();
	}
}

void UAZPlayerFireComponent::OnRep_EquippedWeapons()
{
	AAZWeapon* SubWeapon = GetSubWeapon();
	OnWeaponChanged.Broadcast(CurrentWeapon, SubWeapon, CurrentSlotIndex);
}

// ==================== 입력 처리 ====================

void UAZPlayerFireComponent::InputFireStart(const FInputActionValue& InputValue)
{
	// 구르기 중 사격 불가
	if (OwnerCharacter && OwnerCharacter->MoveComp && OwnerCharacter->MoveComp->bIsRolling)	return;
	// 재장전 중 사격 불가
	if (bIsReloading || bIsSwapping) return;
	// 히트스턴 중 사격 불가
	if (OwnerCharacter && OwnerCharacter->bIsHitStunned) return;
	// 자유 시점 모드에서는 사격 불가
	if (bIsFreeLookMode) return;

	if (AAZPlayerController* PC = Cast<AAZPlayerController>(OwnerCharacter->GetController()))
	{
		if (PC->ItemUsageComp && PC->ItemUsageComp->IsUsingItem()) return;
	}

	if (!CurrentWeapon) return;

	Fire();

	if (CurrentWeapon->FireMode == EFireMode::Auto)
	{
		bIsFiring = true;
		GetWorld()->GetTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&UAZPlayerFireComponent::Fire,
			CurrentWeapon->FireTime,
			true
		);
	}
}

void UAZPlayerFireComponent::InputFireEnd(const FInputActionValue& InputValue)
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
}

void UAZPlayerFireComponent::InputReload(const FInputActionValue& InputValue)
{
	// 자유 시점 모드에서는 재장전 불가
	if (bIsFreeLookMode) return;
	// 구르기 중 재장전 불가
	if (OwnerCharacter && OwnerCharacter->MoveComp && OwnerCharacter->MoveComp->bIsRolling)	return;

	// 이미 재장전 중이면 무시
	if (bIsReloading || bIsSwapping) return;

	// 히트스턴 중 재장전 불가
	if (OwnerCharacter && OwnerCharacter->bIsHitStunned) return;
	
	if (AAZPlayerController* PC = Cast<AAZPlayerController>(OwnerCharacter->GetController()))
	{
		if (PC->ItemUsageComp && PC->ItemUsageComp->IsUsingItem())
		{
			return;
		}
	}

	if (!CurrentWeapon) return;

	// 이미 풀탄이면 무시
	if (CurrentWeapon->CurrentAmmo >= CurrentWeapon->MaxAmmo)
	{
		return;
	}

	Server_Reload();
}

// ==================== 조준 ====================

void UAZPlayerFireComponent::InputAimStart(const FInputActionValue& InputValue)
{
	// 자유 시점 모드에서는 조준 불가
	if (bIsFreeLookMode) return;
	// 구르기 중 조준 불가
	if (OwnerCharacter && OwnerCharacter->MoveComp && OwnerCharacter->MoveComp->bIsRolling)
	{
		return;
	}

	bIsAiming = true;

	// 달리기 중이면 중단
	if (OwnerCharacter && OwnerCharacter->MoveComp && OwnerCharacter->MoveComp->bIsRun)
	{
		OwnerCharacter->MoveComp->bIsRun = false;
		OwnerCharacter->MoveComp->Server_SprintEnd();
	}

	if (OwnerCharacter && OwnerCharacter->bIsHitStunned) return;
	
	// 속도 감소
	if (OwnerCharacter && OwnerCharacter->MoveComp)
	{
		UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
		if (MovementComp)
		{
			float AimSpeed = OwnerCharacter->MoveComp->WalkSpeed * AimingSpeedMultiplier;
			MovementComp->MaxWalkSpeed = AimSpeed;
		}
		OwnerCharacter->MoveComp->SetSpeedMultiplier(AimingSpeedMultiplier);
	}

	if (CrosshairWidget)
	{
		CrosshairWidget->SetCrosshairType(ECrosshairType::Aiming);
	}

	PRINT_LOG(TEXT("Aim Start"));
}

void UAZPlayerFireComponent::InputAimEnd(const FInputActionValue& InputValue)
{
	bIsAiming = false;

	if (OwnerCharacter && OwnerCharacter->MoveComp)
	{
		OwnerCharacter->MoveComp->SetSpeedMultiplier(1.0f);
	}

	if (OwnerCharacter && OwnerCharacter->VignetteMID)
	{
		OwnerCharacter->VignetteMID->SetScalarParameterValue(TEXT("Intensity"), 0.0f);
	}

	if (CrosshairWidget)
	{
		CrosshairWidget->SetCrosshairType(ECrosshairType::Default);
	}

	PRINT_LOG(TEXT("Aim End"));
}

void UAZPlayerFireComponent::UpdateAimZoom(float DeltaTime)
{
	// 1. 유효성 검사 (크래시 방지)
	if (!CamComp || !OwnerCharacter) return;

	// 2. 로컬 플레이어인 경우에만 블러 업데이트 수행
	if (OwnerCharacter->IsLocallyControlled() && OwnerCharacter->VignetteMID)
	{
		// 블러 보간 로직
		float CurrentBlur;
		OwnerCharacter->VignetteMID->GetScalarParameterValue(TEXT("KernelSize"), CurrentBlur);

		float TargetBlur = bIsAiming ? OwnerCharacter->ZoomVignetteIntensity : 0.0f;
		float NewBlur = FMath::FInterpTo(CurrentBlur, TargetBlur, DeltaTime, AimZoomSpeed);

		OwnerCharacter->VignetteMID->SetScalarParameterValue(TEXT("KernelSize"), NewBlur);
	}

	// FOV 보간은 모든 환경(서버/클라)에서 시각적으로 일치해야 하므로 그대로 둡니다.
	float TargetFOV = bIsAiming ? AimingFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CamComp->FieldOfView, TargetFOV, DeltaTime, AimZoomSpeed);
	CamComp->SetFieldOfView(NewFOV);
}

// ==================== 발사 로직 ====================

void UAZPlayerFireComponent::Fire()
{

	if (!CurrentWeapon) return;

	if (OwnerCharacter && OwnerCharacter->bIsDead) return;

	if (CurrentWeapon->CurrentAmmo <= 0)
	{
		CurrentWeapon->PlayEmptySound();
		return;  
	}

	if (bIsReloading || bIsSwapping)	return;
	
	if (!OwnerCharacter || !CurrentWeapon || !CamComp) return;

	// 1단계: 카메라에서 목표 지점 찾기
	FVector CameraLocation = CamComp->GetComponentLocation();
	FVector CameraForward = CamComp->GetForwardVector();

	UE_LOG(LogTemp, Warning, TEXT("[Fire] CamLoc: %s, CamForward: %s"),
		*CameraLocation.ToString(), *CameraForward.ToString());

	FVector MuzzleLocation = CurrentWeapon->GetMuzzleLocation();

	FVector TargetPoint;
	FHitResult CameraHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	Params.AddIgnoredActor(CurrentWeapon);

	FVector TraceEnd = CameraLocation + (CameraForward * CurrentWeapon->MaxDistance);

	if (GetWorld()->LineTraceSingleByChannel(CameraHit, CameraLocation, TraceEnd, ECC_Visibility, Params))
	{
		TargetPoint = CameraHit.ImpactPoint;
	}
	else
	{
		TargetPoint = TraceEnd;
	}

	// 2단계: 목표가 총구 앞에 있는지 확인 (내적)
	FVector MuzzleToTarget = TargetPoint - MuzzleLocation;
	float DotResult = FVector::DotProduct(MuzzleToTarget.GetSafeNormal(), CameraForward);

	FVector FinalDirection;
	if (DotResult > 0.0f)
	{
		FinalDirection = MuzzleToTarget.GetSafeNormal();
	}
	else
	{
		FinalDirection = CameraForward;
	}

	// 탄 퍼짐 적용
	float CurrentSpread = bIsAiming ? AimingSpread : DefaultSpread;
	FinalDirection = ApplySpread(FinalDirection, CurrentSpread);

	// 로컬 연출
	LocalFire(MuzzleLocation, FinalDirection);

	// 서버 동기화
	Server_Fire(MuzzleLocation, FinalDirection);

	// 반동
	ApplyRecoil();

	// 탄약 UI
	BroadcastAmmoChange();
}

void UAZPlayerFireComponent::LocalFire(FVector StartPos, FVector Direction)
{
	if (!CurrentWeapon) return;

	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		// TODO: 카메라 쉐이크
	}

	UAZPlayerAnimInstance* AnimInstance = Cast<UAZPlayerAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance());
	if (AnimInstance)
	{
		AnimInstance->PlayAttackAnim(OwnerCharacter->MoveComp->bIsRun);
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->Fire(StartPos, Direction);
	}
}

// ==================== 네트워크 동기화 ====================

void UAZPlayerFireComponent::Server_Fire_Implementation(FVector_NetQuantize StartPos, FVector_NetQuantizeNormal Direction)
{
	if (OwnerCharacter && OwnerCharacter->bIsDead) return;
	Multicast_Fire(StartPos, Direction);
}

void UAZPlayerFireComponent::Multicast_Fire_Implementation(FVector_NetQuantize StartPos, FVector_NetQuantizeNormal Direction)
{

	if (OwnerCharacter && OwnerCharacter->bIsDead) return;
	// 로컬은 이미 처리함
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled()) return;
	LocalFire(StartPos, Direction);
}

void UAZPlayerFireComponent::Server_Reload_Implementation()
{
	Multicast_Reload();
}

void UAZPlayerFireComponent::Multicast_Reload_Implementation()
{
	if (!CurrentWeapon) return;

	bIsReloading = true;

	// 연사 중이면 중단
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);

	// 애니메이션만 재생 (탄약 충전은 AnimNotify에서)
	UAZPlayerAnimInstance* AnimInstance = Cast<UAZPlayerAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance());
	if (AnimInstance)
	{
		AnimInstance->PlayReloadAnim();
	}
}

// ==================== 반동 ====================

void UAZPlayerFireComponent::ApplyRecoil()
{
	if (!OwnerCharacter) return;

	float PitchRecoil = FMath::RandRange(RecoilPitch * 0.8f, RecoilPitch * 1.2f);
	float YawRecoil = FMath::RandRange(-RecoilYaw, RecoilYaw);

	if (bIsAiming)
	{
		PitchRecoil *= 0.5f;
		YawRecoil *= 0.5f;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (PC)
	{
		PC->AddPitchInput(-PitchRecoil);
		PC->AddYawInput(YawRecoil);
	}

	TargetRecoilOffset.Y -= PitchRecoil * 50.f;
	TargetRecoilOffset.X += YawRecoil * 50.f;
}

void UAZPlayerFireComponent::RecoverRecoil(float DeltaTime)
{
	CurrentRecoilOffset = FMath::Vector2DInterpTo(CurrentRecoilOffset, TargetRecoilOffset, DeltaTime, RecoilRecoverySpeed);
	TargetRecoilOffset = FMath::Vector2DInterpTo(TargetRecoilOffset, FVector2D::ZeroVector, DeltaTime, RecoilRecoverySpeed * 0.5f);

	if (CrosshairWidget)
	{
		CrosshairWidget->SetRecoilOffset(CurrentRecoilOffset);
	}
}

FVector UAZPlayerFireComponent::ApplySpread(FVector Direction, float SpreadAngle)
{
	float SpreadRad = FMath::DegreesToRadians(SpreadAngle);
	return FMath::VRandCone(Direction, SpreadRad);
}

// ==================== 에임 오프셋 ====================

void UAZPlayerFireComponent::UpdateAimOffset(float DeltaTime)
{
	FVector Velocity = OwnerCharacter->GetVelocity();
	Velocity.Z = 0;

	bool bIsInAir = OwnerCharacter->GetCharacterMovement()->IsFalling();
	float Speed = Velocity.Size();

	if (Speed == 0.f && bIsInAir == false)
	{
		float CurrentAimYaw = GetRemoteControlYaw();
		float DeltaYaw = FMath::FindDeltaAngleDegrees(AimOffset_StartYaw, CurrentAimYaw);
		AimOffset_Yaw = DeltaYaw;
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0 || bIsInAir)
	{
		AimOffset_StartYaw = GetRemoteControlYaw();
		AimOffset_Yaw = 0;
		TurnInPlaceType = ETurnInPlace::ETIP_NotTurning;
	}

	AimOffset_Pitch = OwnerCharacter->GetBaseAimRotation().Pitch;

	if (AimOffset_Pitch > 90.f && OwnerCharacter->IsLocallyControlled() == false)
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AimOffset_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimOffset_Pitch);
	}
}

void UAZPlayerFireComponent::TurnInPlace(float DeltaTime)
{
	// 제자리 회전중이 아니라면, AimOffsetYaw값과 보간용 Yaw 값 동기화
	if (TurnInPlaceType == ETurnInPlace::ETIP_NotTurning)
	{
		AimOffset_InterpYaw = AimOffset_Yaw;
	}

	// 현재 에임오프셋의 각도가 일정 각도만큼 벌어졌는지 확인
	if (AimOffset_Yaw < -TurnInPlaceAngle)
	{
		TurnInPlaceType = ETurnInPlace::ETIP_Left;
	}
	else if (AimOffset_Yaw > TurnInPlaceAngle)
	{
		TurnInPlaceType = ETurnInPlace::ETIP_Right;
	}

	// 제자리 회전이 필요한 상태라면 AimOffset 보간 적용
	if (TurnInPlaceType != ETurnInPlace::ETIP_NotTurning)
	{
		// -90 있던던 AimOffset 값을 0 에 가깝게 만들자.
		AimOffset_InterpYaw = FMath::FInterpTo(AimOffset_InterpYaw, 0.f, DeltaTime, TurnInPlaceTime);
		AimOffset_Yaw = AimOffset_InterpYaw;

		// -3도도 체크하기 위해서
		if (FMath::Abs(AimOffset_Yaw) < TurnInPlaceEndAngle)
		{
			TurnInPlaceType = ETurnInPlace::ETIP_NotTurning;
			AimOffset_StartYaw = GetRemoteControlYaw();
		}
	}
}

float UAZPlayerFireComponent::GetRemoteControlYaw()
{
	if (OwnerCharacter->IsLocallyControlled() == false)
	{
		return FRotator::NormalizeAxis(OwnerCharacter->GetActorRotation().Yaw);
	}
	return OwnerCharacter->GetControlRotation().Yaw;
}


void UAZPlayerFireComponent::StartThrowGrenade_Server_Implementation(int32 ItemID)
{
	ThrowableItemID = ItemID;
	StartThrowGrenade_Multicast(ItemID);
}

void UAZPlayerFireComponent::StartThrowGrenade_Multicast_Implementation(int32 ItemID)
{
	AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(GetOwner());
	if (Player == nullptr) return;
	if (UAZPlayerAnimInstance* AnimInst = Cast<UAZPlayerAnimInstance>(Player->GetMesh()->GetAnimInstance()))
	{
		if (AnimInst->PlayGrenadeAnim())
		{
			APawn* OwnerPawn = Cast<APawn>(GetOwner());
			if (OwnerPawn)
			{
				AAZPlayerController* PC = Cast<AAZPlayerController>(OwnerPawn->GetController());
				if (PC && PC->EquipmentComp)
				{
					PC->EquipmentComp->SubUsedQuickSlotItem();
					PC->EquipmentComp->SetUsedQuickSlotIndex(ESlotIndex::Slot_0);

				}
			}
		}
	}
}


void UAZPlayerFireComponent::SpawnGrenadeInHand()
{
	if (!GetOwner()->HasAuthority()) return;
	SpawnGrenadeProjectile_Server();
}

void UAZPlayerFireComponent::ExecuteThrowGrenade()
{
	if (!GetOwner()->HasAuthority()) return;

	ExecuteThrowGrenadeProjectile_Server();
}

void UAZPlayerFireComponent::SpawnGrenadeProjectile_Server_Implementation()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr) return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr) return;

	const FAZThrowablesItemDataTable* ThrowableData = DataManger->GetItemDataByID<FAZThrowablesItemDataTable>(ThrowableItemID);
	if (ThrowableData == nullptr) return;

	if (GrenadeClass)
	{
		AAZPlayerCharacter* Character = Cast<AAZPlayerCharacter>(GetOwner());
		if (!Character) return;

		FVector SpawnLoc;
		FRotator SpawnRot = Character->GetController()->GetControlRotation();

		FName SocketName = TEXT("Throw_Socket");
		if (Character->GetMesh()->DoesSocketExist(SocketName))
		{
			SpawnLoc = Character->GetMesh()->GetSocketLocation(SocketName);
		}
		else
		{
			SpawnLoc = Character->GetActorLocation() + (SpawnRot.Vector() * 50.f);
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		Grenade = GetWorld()->SpawnActor<AAZThrowProjectile>(GrenadeClass, SpawnLoc, SpawnRot, SpawnParams);

		if (Grenade)
		{
			if (GetOwner())
			{
				Grenade->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
				Grenade->AddIgnoreActor(GetOwner());
				Grenade->SetHeldState();
				Grenade->InitializeProjectile(ThrowableData->Max_Damage);
				ThrowableItemID = 0;
			}
		}

	}
}

void UAZPlayerFireComponent::ExecuteThrowGrenadeProjectile_Server_Implementation()
{
	if (Grenade)
	{
		AAZPlayerCharacter* Character = Cast<AAZPlayerCharacter>(GetOwner());
		if (Character)
		{
			FRotator ControlRotation = Character->GetController()->GetControlRotation();
			FVector LaunchDirection = ControlRotation.Vector();
			//Grenade->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			Grenade->LaunchProjectile(LaunchDirection);

			Grenade = nullptr;
		}
	}
}

void UAZPlayerFireComponent::SetFreeLookMode(bool bEnable)
{
	bIsFreeLookMode = bEnable;

	if (bEnable)
	{
		// Lobby 모드
		ForceEndAim();
		CancelReload();
		InputFireEnd(FInputActionValue());  // 연사 중단

		// 크로스헤어 숨기기
		if (CrosshairWidget)
		{
			CrosshairWidget->SetVisibility(ESlateVisibility::Collapsed);
			UE_LOG(LogTemp, Warning, TEXT("[FireComp] Crosshair HIDDEN"));
		}

		// 무기 숨기기
		if (CurrentWeapon)
		{
			CurrentWeapon->SetActorHiddenInGame(true);
			
		}
	}
	else
	{
		// Ingame 모드
		if (CrosshairWidget)
		{
			CrosshairWidget->SetVisibility(ESlateVisibility::Visible);
			UE_LOG(LogTemp, Warning, TEXT("[FireComp] Crosshair VISIBLE"));
		}

		// 무기 보이기
		if (CurrentWeapon)
		{
			CurrentWeapon->SetActorHiddenInGame(false);
			
		}
	}
}