// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/AZWeaponTypes.h"
#include "AZWeapon.generated.h"

class UNiagaraSystem;
class USoundBase;
class UMaterialInterface;
class AAZBullet;

UCLASS()
class ASHZERO_API AAZWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:
    // ==================== 컴포넌트 ====================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Weapon")
    class USkeletalMeshComponent* WeaponMesh;

    // ==================== 무기 타입 ====================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Weapon")
    EWeaponType WeaponType = EWeaponType::Pistol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Weapon")
    EFireMode FireMode = EFireMode::Single;

    // ==================== 레벨 시스템 (BP에서 설정) ====================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Level")
    TArray<FWeaponLevelData> LevelDataArray;

	// 현재 무기 레벨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentLevel, Category = "AZ|Levels", meta = (ClampMin = "0"))
    int32 CurrentLevel = 0;

    // ==================== 현재 적용된 스탯 (런타임) ====================
    UPROPERTY(BlueprintReadOnly, Category = "AZ|Stat")
	float Damage;                       // 데미지

    UPROPERTY(BlueprintReadOnly, Category = "AZ|Stat")
	float FireTime;                     // 발사 속도

    UPROPERTY(BlueprintReadOnly, Category = "AZ|Stat")
	float MaxDistance;                  // 최대 사거리

    UPROPERTY(BlueprintReadOnly, Category = "AZ|Stat")
	float ReloadTime;                   // 재장전 시간

    UPROPERTY(BlueprintReadOnly, Category = "AZ|Stat")
	int32 MaxAmmo;                      // 최대 탄약

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "AZ|Stat")
	int32 CurrentAmmo;                  // 현재 탄약

    // ==================== Projectile ====================
    UPROPERTY(EditDefaultsOnly, Category = "AZ|Weapon")
	TSubclassOf<AAZBullet> BulletClass; // 탄환 클래스


    // ==================== 이펙트 ====================
    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	UNiagaraSystem* MuzzleFlashEffect;  // 총구 화염 이펙트

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	FRotator MuzzleFlashRotationOffset = FRotator::ZeroRotator; // 총구 화염 회전 오프셋

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	FVector MuzzleFlashScale = FVector(1.f, 1.f, 1.f);  // 총구 화염 스케일

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	UNiagaraSystem* ImpactEffect;       // 명중 이펙트

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	UNiagaraSystem* BulletTrailEffect;  // 탄환 궤적 이펙트

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	FRotator BulletTrailRotationOffset = FRotator::ZeroRotator; //  탄환 궤적 회전 오프셋

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	FVector BulletTrailScale = FVector(1.f, 1.f, 1.f);  // 탄환 궤적 스케일

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	UNiagaraSystem* CriticalHitEffect;  // 크리티컬 히트 이펙트

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
    UNiagaraSystem* ExecuteEffect;      // 처형 이펙트

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	UNiagaraSystem* LifeStealEffect;    // 생명력 흡수 이펙트

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	UNiagaraSystem* ExplosionEffect;    // 폭발 이펙트

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	USoundBase* FireSound;              // 발사 사운드

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	USoundBase* ImpactSound;            // 명중 사운드

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	USoundBase* CriticalHitSound;       // 크리티컬 히트 사운드

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	USoundBase* ReloadSound;            // 재장전 사운드

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	USoundBase* EmptyFireSound;         // 빈 탄창 발사 사운드

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	UMaterialInterface* BulletDecalMaterial;// 탄환 데칼 머티리얼

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	FVector DecalSize = FVector(5.f, 5.f, 5.f);// 데칼 크기

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	float DecalLifetime = 10.0f;        // 데칼 지속 시간

    // ==================== UI ====================

    UPROPERTY(EditDefaultsOnly, Category = "AZ|UI")
	UTexture2D* WeaponIcon;             // 무기 아이콘

    UPROPERTY(EditDefaultsOnly, Category = "AZ|UI")
	FText WeaponName;                   // 무기 이름

    // ==================== 레벨 시스템 함수 ====================

    UFUNCTION(BlueprintCallable, Category = "AZ|Level")
	void SetLevel(int32 NewLevel);      //레벨 설정

    UFUNCTION(BlueprintPure, Category = "AZ|Level")
	FWeaponLevelData GetCurrentLevelData() const;   // 현재 레벨 데이터

    UFUNCTION(BlueprintPure, Category = "AZ|Level")
	int32 GetMaxLevel() const { return FMath::Max(0, LevelDataArray.Num() - 1); }   // 최대 레벨 반환

    UFUNCTION(BlueprintPure, Category = "AZ|Level")
	bool HasEffect(EWeaponEffect EffectType) const; // 특정 효과가 있는지 확인

    UFUNCTION(BlueprintPure, Category = "AZ|Level")
	FWeaponEffectData GetEffectData(EWeaponEffect EffectType) const;    // 특정 효과 데이터 반환

    // ==================== 기본 함수 ====================
	void Fire(FVector StartPos, FVector Direction);             // 발사 함수
	void FireLineTrace(FVector StartPos, FVector Direction);    // 라인 트레이스 발사
	void FireBullet(FVector StartPos, FVector Direction);       // 탄환 발사
	void Reload();      // 재장전 함수
	void Equip(ACharacter* Character);  // 무기 장착 함수
	void PlayEmptySound();      // 빈 탄창 발사 사운드 재생

	FVector GetMuzzleLocation();    // 총구 위치 반환

    UFUNCTION(BlueprintPure, Category = "Weapon")
    float GetFireTime() const { return FireTime; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool CanFire() const { return CurrentAmmo > 0; }

protected:
    UFUNCTION()
	void OnRep_CurrentLevel();      // CurrentLevel 복제 시 호출
	void ApplyLevelData();          // 레벨 데이터 적용
	void ProcessSingleHit(const FHitResult& HitResult, float DamageMultiplier, AController* InstigatorController);  // 단일 피격 처리
	void ProcessHitEffects(AActor* HitActor, float& OutDamage, const FHitResult& HitResult);    // 피격 이펙트 처리
	void ApplyStatusEffectsToTarget(AActor* Target);    // 상태 이상 효과 적용
	void ProcessKillEffects(AActor* KilledActor);       // 처치 이펙트 처리
	void ApplyExplosion(const FVector& Location, float Radius, float DamageRatio);      // 폭발 효과 적용

private:
    // 이펙트 재생
	void PlayFireEffects(FVector MuzzleLocation, FVector Direction);       // 발사 이펙트
	void PlayImpactEffects(const FHitResult& HitResult);    // 명중 이펙트
	void PlayTrailEffect(const FVector& Start, const FVector& End); // 탄환 궤적 이펙트
	void PlayLifeStealEffects();                        // 생명력 흡수 이펙트
	void PlayExecuteEffects(const FVector& Location);   // 처형 이펙트
	void PlayCriticalEffects(const FVector& Location);      // 크리티컬 이펙트
	void PlayExplosionEffects(const FVector& Location);     // 폭발 이펙트
    
    // 크리티컬 체크
    bool CheckCritical(float& OutDamage);

    // 스택 관리
    UPROPERTY()
    TMap<AActor*, FEnemyStackInfo> EnemyStacks;

    int32 AddStackToEnemy(AActor* Enemy);
    void ClearExpiredStacks();

    UPROPERTY(EditDefaultsOnly, Category = "AZ|Settings")
    float StackExpireTime = 5.0f;

};
