// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AZSpawnProjectile.generated.h"

/**
 * 
 */
UCLASS()
class ASHZERO_API UAZSpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	// ===== 필수 설정 =====
	// 스폰할 발사체 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Projectile")
	TSubclassOf<class AAZProjectile> ProjectileClass;

	// 스폰 위치 소켓
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Projectile")
	FName SpawnSocket = TEXT("Muzzle_Front");

	/*
	// ===== 데미지 오버라이드 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Stats")
	bool bOverrideDamage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Stats", meta = (EditCondition = "bOverrideDamage"))
	float CustomDamage = 15.0f;

	// ===== 속도 오버라이드 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Stats")
	bool bOverrideSpeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Stats", meta = (EditCondition = "bOverrideSpeed"))
	float CustomSpeed = 2000.0f;

	// ===== 중력 오버라이드 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Stats")
	bool bOverrideGravity = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Stats", meta = (EditCondition = "bOverrideGravity"))
	float CustomGravityScale = 1.0f;

	// ===== 착탄 효과 오버라이드 ===== 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Impact")
	bool bOverrideImpact = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Impact", meta = (EditCondition = "bOverrideImpact"))
	bool bSpawnImpactActor = false;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Impact", meta = (EditCondition = "bOverrideImpact && bSpawnImpactActor"))
	TSubclassOf<AActor> ImpactActorClass;
	*/
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

};
