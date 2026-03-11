// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AZPlayerAnimInstance.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/AZPlayerFireComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System/AZSessionSubsystem.h"
#include "Components/AZPlayerMoveComponent.h"
#include "Weapon/AZWeapon.h"


void UAZPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);


	APawn* OwnerPawn = TryGetPawnOwner();
	AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(OwnerPawn);
	if (!Player) return;

	if (!Player->MoveComp) return;

	if (Player->MoveComp)
	{
		bIsFreeLookMode = Player->MoveComp->bIsFreeLookMode;
	}
	Velocity = Player->GetVelocity();
	FVector ForwardVector = Player->GetActorForwardVector();

	// Velocity 자체의 스피드
	Speed = Velocity.Length();

	// 가고자 하는 방향 Velocity는 월드 기준.
	// 월드 기준이 아니라, 플레이어 로컬공간의 회전값을 얻기위해 역행렬을 곱해준다.
	Direction = Player->GetActorTransform().InverseTransformVector(Velocity).Rotation().Yaw;


	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	bIsInAir = Movement->IsFalling();
	bIsAccelerating = Movement->GetCurrentAcceleration().Size() > 0.f ? true : false;
	if (!Player->FireComp || !IsValid(Player->FireComp)) return;
	// Player 컴포넌트에서 계산된 AO 값 애니메이션에 전달
	AO_Yaw = Player->FireComp->AimOffset_Yaw;
	AO_Pitch = Player->FireComp->AimOffset_Pitch;

	// TurnInPlace 동기화
	TurnInPlaceType = Player->FireComp->TurnInPlaceType;
		

	
	// 왼손이 있어야하는 위치를 계산한다.
	// Weapon 값이 nullptr 체크 + 변수로 활용
	if (AAZWeapon* Weapon = Player->FireComp->CurrentWeapon) // 무기가 있는지 확인
	{
		CurrentWeaponType = Weapon->WeaponType;
		WeaponAnimIndex = static_cast<int32>(Weapon->WeaponType);


		// 총기의 왼손 위치 소켓을 가져온다.
		LeftHandTransfrom = Weapon->WeaponMesh->GetSocketTransform(TEXT("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		// 캐릭터 기준으로 변환된 위치와 회전값
		FVector Position;
		FRotator Rotator;

		Player->GetMesh()->TransformToBoneSpace(
			FName("Hand_R"),
			LeftHandTransfrom.GetLocation(),
			FRotator::ZeroRotator,
			Position,
			Rotator);

		// hand_r 공간으로 변환된 로테이션
		LeftHandTransfrom.SetLocation(Position);
		LeftHandTransfrom.SetRotation(FQuat(Rotator));
	}

}

void UAZPlayerAnimInstance::PlayAttackAnim(bool bIsRun)
{
	if (AttackAnimMontages.IsValidIndex(WeaponAnimIndex) && AttackAnimMontages[WeaponAnimIndex])
	{
		Montage_Play(AttackAnimMontages[WeaponAnimIndex]);
	}
}

void UAZPlayerAnimInstance::PlayReloadAnim()
{
	if (ReloadAnimMontages.IsValidIndex(WeaponAnimIndex) && ReloadAnimMontages[WeaponAnimIndex])
	{
		Montage_Play(ReloadAnimMontages[WeaponAnimIndex]);
	}
}

void UAZPlayerAnimInstance::PlayWeaponSwapAnim()
{
	if (WeaponSwapMontage.IsValidIndex(WeaponAnimIndex) && WeaponSwapMontage[WeaponAnimIndex])
	{
		Montage_Play(WeaponSwapMontage[WeaponAnimIndex]);
	}
}

void UAZPlayerAnimInstance::PlayDeathAnim()
{
	if (DeathAnimMontage)
	{
		Montage_Play(DeathAnimMontage);
	}
}

void UAZPlayerAnimInstance::PlayRollAnim()
{
	if (!RollAnimMontage) return;

	Montage_Play(RollAnimMontage);
	
}
bool UAZPlayerAnimInstance::PlayUseItemAnim()
{
	if (SuppliesAnimMontage && !IsPlayingActionMontage())
	{
		Montage_Play(SuppliesAnimMontage);
		return true;
	}
	return false;
}

bool UAZPlayerAnimInstance::PlayGrenadeAnim()
{
	if (GrenadeAnimMontage && !IsPlayingActionMontage())
	{
		Montage_Play(GrenadeAnimMontage);
		return true;
	}
	return false;
}

bool UAZPlayerAnimInstance::IsPlayingActionMontage() const
{
	// Reload, Roll, Death 등 액션 몽타주 재생 중인지 체크
	if (Montage_IsPlaying(RollAnimMontage) || Montage_IsPlaying(DeathAnimMontage) || Montage_IsPlaying(GrenadeAnimMontage))
	{
		return true;
	}

	// 재장전 배열 체크
	for (UAnimMontage* ReloadMontage : ReloadAnimMontages)
	{
		if (ReloadMontage && Montage_IsPlaying(ReloadMontage))
		{
			return true;
		}
	}

	for (const UAnimMontage* SwapMontage : WeaponSwapMontage)
	{
		if (SwapMontage && Montage_IsPlaying(SwapMontage))
		{
			return true;
		}
	}
	// 필요한 몽타주 추가
	return false;
}