// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Util/AZDefine.h"
#include "AZThrowProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class ASHZERO_API AAZThrowProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAZThrowProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "AZ|Projectile")
	TObjectPtr<USphereComponent> CollisionComp;

	// ¼ö·ùÅº ¸Þ½¬
	UPROPERTY(VisibleDefaultsOnly, Category = "AZ|Projectile")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AZ|Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(Server, Unreliable)
	void Explode_Server(FVector ImpactLocation);

	UFUNCTION(NetMulticast, Unreliable)
	void ExplodeFX_Multicast(FVector Location);

	UFUNCTION(NetMulticast, Unreliable)
	void PlaySFX_Multicast(ESFXType SFXType);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetVelocity(FVector Direction);

	void InitializeProjectile(float InDamage);

	void AddIgnoreActor(AActor* ActorToIgnore);

	void SetHeldState();

	void LaunchProjectile(FVector LaunchDirection);

protected:
	float ExplosionDamage = 50.f;
	UPROPERTY(EditAnywhere, Category = "AZ|Input")
	float ExplosionRadius = 300.f;
	UPROPERTY(EditAnywhere, Category = "AZ|Input")
	bool bShowDrawDebug = false;
	UPROPERTY(EditDefaultsOnly, Category = "AZ|Effects")
	UNiagaraSystem* ExplosionVFX;

	FRotator TumblingRotationRate;

	bool bIsThrowing = false;

	
};
