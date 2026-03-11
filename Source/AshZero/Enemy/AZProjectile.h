// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AZProjectile.generated.h"

class UNiagaraSystem;

UCLASS()
class ASHZERO_API AAZProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void Explosion(const FVector& Location, const FVector& ImpactNormal);

public:
	// ===== 컴포넌트 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Components")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Components")
	class UProjectileMovementComponent* ProjectileMovement;

	// ===== 기본 설정 (BP에서 설정) =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Projectile")
	float Damage = 10.0f;
	
	//발사체 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Projectile")
	float Speed = 2000.0f;

	// 중력 영향도 (0.0f = 무중력, 1.0f = 기본 중력)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Projectile")
	float GravityScale = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Projectile")
	float LifeSpan = 5.0f;	// 수명

	// ===== 착탄 효과 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Impact")
	bool bSpawnImpactActor = false;

	// 착탄 시 스폰할 액터 (독 지대 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Impact", meta = (EditCondition = "bSpawnImpactActor"))
	TSubclassOf<AActor> ImpactActorClass;

	// ===== 폭발(AoE) 설정 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Impact")
	bool bExplosionProjectile = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Explosion")
	float ExplosionDamage = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Explosion")
	float ExplosionRadius = 300.0f; // 데미지 범위

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Explosion")
	float ExplosionDelay = 0.0f; // 폭발 지연 시간 (밀리초)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Explosion")
	class UNiagaraSystem* ExplosionEffect; // 폭발 비주얼 이펙트

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Explosion")
	class USoundBase* ExplosionSound; // 폭발 사운드

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Debug")
	bool bDebugDraw = false;

	// ===== 함수 =====
	UFUNCTION(BlueprintCallable, Category = "AZ|Projectile")
	void Launch(FVector Direction);

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	void SpawnImpactActor(const FVector& Location, const FRotator& Rotation);

	FTimerHandle ExplosionTimerHandle;

	FVector StoredExplosionLocation;
	FVector StoredImpactNormal;

	void ExecuteExplosion();

};
