
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Util/AZDefine.h"
#include "Weapon/AZWeapon.h"
#include "AZPlayerAnimInstance.generated.h"

class UAnimMontage;
/**
 * 
 */
UCLASS()
class ASHZERO_API UAZPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// 플레이어 이동 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	float Speed = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	float Direction = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	bool bIsInAir = false;

	// 입력이 있는 상태로 이동중인지 판단은 velocity 말고 accel(가속도)값을 사용하자.
	UPROPERTY(BlueprintReadWrite, Category = "AZ|Value")
	bool bIsAccelerating = false;

	// C++ 클래스에 Velocity변수를 추가하고 연동한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	FVector Velocity;

	// 공격 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Anim")
	TArray<class UAnimMontage*> AttackAnimMontages;

	// 재장전 애니메이션
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Anim")
	TArray<class UAnimMontage*> ReloadAnimMontages;

	// 무기 스왑 애니메이션 
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Anim")
	TArray<class UAnimMontage*> WeaponSwapMontage;

	// 죽음 애니메이션
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Anim")
	TObjectPtr<UAnimMontage> DeathAnimMontage;

	// 구르기 애니메이션
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Anim")
	TObjectPtr<UAnimMontage> RollAnimMontage;

	// 소모품 애니메이션
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Anim")
	TObjectPtr<UAnimMontage> SuppliesAnimMontage;

	// 수류탄 애니메이션
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Anim")
	TObjectPtr<UAnimMontage> GrenadeAnimMontage;

	// 액션 몽타주 재생 중인지 체크
	UFUNCTION(BlueprintCallable, Category = "AZ|Animation")
	bool IsPlayingActionMontage() const;

	// 에임 오프셋값
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	float AO_Yaw = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	float AO_Pitch = 0;

	// 왼손이 있어야하는 Transform 을 역으로 계산
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	FTransform LeftHandTransfrom;

	// 현재 무기 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	EWeaponType CurrentWeaponType = EWeaponType::Pistol;

	// 무기 애니메이션 인덱스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	int32 WeaponAnimIndex = 0;

	// 피격 리액션 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	EHitReactionType HitReactionType = EHitReactionType::LightHit;

	// 넉백 피격 시 적용되는 힘의 세기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (EditCondition = "HitReactionType == EHitReactionType::Knockback"))
	float KnockbackForce = 800.0f;

	// 공격 애니메이션 재생 함수
	void PlayAttackAnim(bool bIsRun);
	//재장전 애니메이션
	void PlayReloadAnim();
	// 무기 스왑 애니메이션
	void PlayWeaponSwapAnim();
	// 죽음 애니메이션 재생 함수
	void PlayDeathAnim();
	// 죽음 애니메이션 재생 함수
	void PlayRollAnim();
	// 소모품 애니메이션 재생 함수
	bool PlayUseItemAnim();
	// 수류탄 애니메이션 재생 함수
	bool PlayGrenadeAnim();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AZ|Value")
	ETurnInPlace TurnInPlaceType = ETurnInPlace::ETIP_NotTurning;

	UPROPERTY(BlueprintReadOnly, Category = "AZ|Anim")
	bool bIsFreeLookMode = false;
};
