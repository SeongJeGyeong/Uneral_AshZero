// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AZProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AAZProjectile::AAZProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(10.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComp;

	/*
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;

	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 1000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bSimulationEnabled = true;
	*/
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->bRotationFollowsVelocity = true;

}

void AAZProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AAZProjectile::OnOverlap);
	CollisionComp->OnComponentHit.AddDynamic(this, &AAZProjectile::OnHit);

	ProjectileMovement->Deactivate();
	SetLifeSpan(LifeSpan);
}

void AAZProjectile::Launch(FVector Direction)
{
	
	if (!ProjectileMovement) return;

	// 설정
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->ProjectileGravityScale = GravityScale;
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * Speed;

	ProjectileMovement->Activate();
}

void AAZProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetOwner() && OtherActor != this)
	{
		if(bExplosionProjectile)
		{
			Explosion(SweepResult.ImpactPoint, SweepResult.ImpactNormal);
			return;
		}
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), GetOwner(), UDamageType::StaticClass());
	}
}

void AAZProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		if(bExplosionProjectile)
		{
			// 폭발형 투사체인 경우, 폭발 처리
			Explosion(Hit.ImpactPoint, Hit.ImpactNormal);

			return;
		}
		// 면의 법선(ImpactNormal)을 기준으로 기울기 회전값 계산
		FRotator SpawnRotation = FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator();

		SpawnImpactActor(Hit.ImpactPoint, SpawnRotation);
		Destroy();
	}
}

void AAZProjectile::SpawnImpactActor(const FVector& Location, const FRotator& Rotation)
{
	if (!bSpawnImpactActor || !ImpactActorClass) return;
	if (!HasAuthority()) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 바닥에 스폰하도록 Z는 약간 위로
	FVector SpawnLocation = Location + FVector(0.0f, 0.0f, 5.0f);

	GetWorld()->SpawnActor<AActor>(ImpactActorClass, SpawnLocation, Rotation, SpawnParams);
}

void AAZProjectile::Explosion(const FVector& Location, const FVector& ImpactNormal)
{
	if (!HasAuthority()) return;
	if (!bExplosionProjectile) return;

	// 1. 위치 정보 저장
	StoredExplosionLocation = Location;
	StoredImpactNormal = ImpactNormal;

	if (ExplosionDelay <= 0.0f)
	{
		// 지연 시간이 없으면 즉시 실행
		ExecuteExplosion();
	}
	else
	{
		// 지연 시간이 있으면 투사체를 멈추고 타이머 시작
		ProjectileMovement->StopMovementImmediately();

		// 투사체가 공중에 멈춰있거나 바닥에 고정되게 함
		SetActorLocation(Location);

		// 충돌체 비활성화 (이미 닿았으니 더 이상의 충돌 이벤트 방지)
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetWorldTimerManager().SetTimer(
			ExplosionTimerHandle,
			this,
			&AAZProjectile::ExecuteExplosion,
			ExplosionDelay,
			false
		);

	}

}

void AAZProjectile::ExecuteExplosion()
{


	// 1. 범위 데미지 적용 (ApplyRadialDamage)
	// 위치, 반경, 데미지량, 데미지 타입, 무시할 액터들, 가해자 등 설정
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	IgnoreActors.Add(GetOwner()); // 보스 자신은 데미지 안 받게

	UGameplayStatics::ApplyRadialDamage(
		this,
		ExplosionDamage,
		StoredExplosionLocation,
		ExplosionRadius,
		UDamageType::StaticClass(),
		IgnoreActors,
		this,
		GetInstigatorController(),
		true // bDoFullDamage: 중심부와 외곽 데미지를 동일하게 할지 여부 
	);

	if (bDebugDraw)
	{
		// 12: 구체를 구성하는 선의 개수 (클수록 부드러움)
		// 2.0f: 지속 시간 (요청하신 2초)
		// 1.5f: 선의 두께 (너무 얇으면 안 보여서 1.5 정도로 설정)
		DrawDebugSphere(GetWorld(), StoredExplosionLocation, ExplosionRadius, 36, FColor::Red, false, 3.0f, 0, 3.5f);

		// --- 누구한테 닿았는지 디버그 로직 추가 ---
		TArray<FOverlapResult> OverlapResults;
		FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(GetOwner());

		// 범위 내에 있는 모든 물체 감지 (Pawn 등)
		bool bHit = GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			StoredExplosionLocation,
			FQuat::Identity,
			ECC_Pawn, // 캐릭터 위주로 체크
			SphereShape,
			QueryParams
		);

		if (bHit)
		{
			for (const FOverlapResult& Result : OverlapResults)
			{
				if (AActor* HitActor = Result.GetActor())
				{
					// 노란색으로 로그 출력: [폭발] 대상 이름 (거리)
					float Distance = FVector::Dist(StoredExplosionLocation, HitActor->GetActorLocation());
					UE_LOG(LogTemp, Warning, TEXT("=== EXPLOSION HIT: [%s] / Distance: %.2f ==="), *HitActor->GetName(), Distance);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Explosion occurred, but NO ONE was in range."));
		}

	}

	// 2. 이펙트 및 사운드 재생
	if (ExplosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionEffect, StoredExplosionLocation, StoredImpactNormal.Rotation());
	}
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, StoredExplosionLocation);
	}

	// 3. 기존에 있던 장판 생성 로직 호출 (필요한 경우)
	if (bSpawnImpactActor)
	{
		FRotator SpawnRotation = FRotationMatrix::MakeFromZ(StoredImpactNormal).Rotator();
		SpawnImpactActor(StoredExplosionLocation, SpawnRotation);
	}

	// 4. 투사체 파괴
	Destroy();
}
