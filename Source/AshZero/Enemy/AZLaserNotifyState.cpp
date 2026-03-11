// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/AZLaserNotifyState.h"
#include "Enemy/AZBossBase.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/SplineComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "AshZero.h"

UAZLaserNotifyState::UAZLaserNotifyState()
{
	MaxLaserRange = 3000.0f;
	LaserRadius = 30.0f;
	DamagePerSecond = 50.0f;
	DamageInterval = 0.1f;
	TargetHeightOffset = 60.0f;
}

void UAZLaserNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner()) return;

	OwnerBoss = Cast<AAZBossBase>(MeshComp->GetOwner());
	if (!OwnerBoss) return;

	// 플레이어 가슴 높이 저장 (발사 시점에 고정)
	AActor* Target = OwnerBoss->GetCurrentTarget();
	if (Target)
	{
		TargetLaserHeight = Target->GetActorLocation().Z + TargetHeightOffset;
	}
	else
	{
		TargetLaserHeight = OwnerBoss->GetActorLocation().Z + TargetHeightOffset;
	}

	// 서버에서만 VFX 스폰
	//if (!OwnerBoss->HasAuthority()) return;

	FVector StartPos = MeshComp->GetSocketLocation(LaserSocketName);
	FVector ForwardDir = OwnerBoss->GetActorForwardVector();
	FVector EndPos = StartPos + ForwardDir * MaxLaserRange;
	EndPos.Z = TargetLaserHeight;

	// ===== 방식 1: 나이아가라 직접 스폰 =====
	if (BeamEffect)
	{
		BeamComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			BeamEffect,
			MeshComp,
			LaserSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false
		);

		if (BeamComponent)
		{
			BeamComponent->ComponentTags.Add(FName("LaserBeam"));
			BeamComponent->SetVectorParameter(BeamStartParamName, StartPos);
			BeamComponent->SetVectorParameter(BeamEndParamName, EndPos);
		}
	}

	// ===== 방식 2: 스플라인 BP 스폰 =====
	if (BeamVFXClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerBoss;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		BeamVFXActor = MeshComp->GetWorld()->SpawnActor<AActor>(
			BeamVFXClass,
			StartPos,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (BeamVFXActor)
		{
			BeamSpline = BeamVFXActor->FindComponentByClass<USplineComponent>();

			if (BeamSpline)
			{
				BeamSpline->SetWorldLocationAtSplinePoint(0, StartPos);
				BeamSpline->SetWorldLocationAtSplinePoint(1, EndPos);
				BeamSpline->UpdateSpline();

				PRINT_LOG(TEXT("Beam VFX Spawned - Start: %s, End: %s"), *StartPos.ToString(), *EndPos.ToString());
			}
		}
	}

	// 타이머 초기화
	DamageTimer = 0.0f;
	DamagedActorsThisTick.Empty();

	PRINT_LOG(TEXT("Laser NotifyState Begin - Target Height: %f"), TargetLaserHeight);
}

void UAZLaserNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (!OwnerBoss || !MeshComp) return;

	AAZBossBase* Boss = Cast<AAZBossBase>(MeshComp->GetOwner());
	if (!Boss) return;

	FVector LaserStart = MeshComp->GetSocketLocation(LaserSocketName);
	FVector ForwardDir = Boss->GetActorForwardVector();
	FVector LaserEnd = LaserStart + ForwardDir * MaxLaserRange;
	LaserEnd.Z = TargetLaserHeight;

	UpdateBeamEffect(LaserStart, LaserEnd);

	// 서버에서만 데미지 처리
	if (OwnerBoss->HasAuthority())
	{
		PerformLaserTrace(MeshComp, Boss, FrameDeltaTime);
	}
}

void UAZLaserNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp)
	{
		TArray<USceneComponent*> Children;
		MeshComp->GetChildrenComponents(true, Children);

		for (USceneComponent* Child : Children)
		{
			if (UNiagaraComponent* NC = Cast<UNiagaraComponent>(Child))
			{
				if (NC->ComponentHasTag(FName("LaserBeam")))
				{
					NC->DeactivateImmediate();
					NC->DestroyComponent();
				}
			}
		}
	}

	// 나이아가라 정리
	if (BeamComponent)
	{
		BeamComponent->DestroyComponent();
		BeamComponent = nullptr;
	}
	// 스플라인 BP 정리
	if (BeamVFXActor)
	{
		BeamVFXActor->Destroy();
		BeamVFXActor = nullptr;
		BeamSpline = nullptr;
	}
	if (ImpactComponent)
	{
		ImpactComponent->DeactivateImmediate();
		ImpactComponent->DestroyComponent();
		ImpactComponent = nullptr;
	}
	BeamComponent = nullptr;
	OwnerBoss = nullptr;
	DamagedActorsThisTick.Empty();

	PRINT_LOG(TEXT("Laser NotifyState End"));
}

void UAZLaserNotifyState::PerformLaserTrace(USkeletalMeshComponent* MeshComp, AAZBossBase* Boss, float DeltaTime)
{
	if (!OwnerBoss || !MeshComp) return;

	// 레이저 시작점 (소켓 위치)
	FVector LaserStart = MeshComp->GetSocketLocation(LaserSocketName);

	// 정면 방향으로 레이저 끝점 계산
	FVector ForwardDir = OwnerBoss->GetActorForwardVector();
	FVector LaserEnd = LaserStart + ForwardDir * MaxLaserRange;

	// Z 높이만 플레이어 가슴 높이로 고정
	LaserEnd.Z = TargetLaserHeight;

	// 실제 레이저 방향 (시작점 → 끝점)
	FVector LaserDirection = (LaserEnd - LaserStart).GetSafeNormal();

	// LineTrace (또는 SphereTrace for 굵기)
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerBoss);
	QueryParams.bTraceComplex = false;

	bool bHit;
	FVector ActualEnd = LaserEnd;

	if (LaserRadius > 0.0f)
	{
		// Sphere Trace (굵기 있는 레이저)
		bHit = MeshComp->GetWorld()->SweepSingleByChannel(
			HitResult,
			LaserStart,
			LaserEnd,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(LaserRadius),
			QueryParams
		);
	}
	else
	{
		// Line Trace (얇은 레이저)
		bHit = MeshComp->GetWorld()->LineTraceSingleByChannel(
			HitResult,
			LaserStart,
			LaserEnd,
			ECC_Pawn,
			QueryParams
		);
	}

	// 충돌 지점 또는 최대 사거리
	if (bHit)
	{
		ActualEnd = HitResult.ImpactPoint;
	}

	// 나이아가라 업데이트
	UpdateBeamEffect(LaserStart, ActualEnd);

	// 디버그 드로우
	if (bDebugDraw)
	{
		DrawDebugLine(MeshComp->GetWorld(), LaserStart, ActualEnd, FColor::Green, false, 0.1f, 0, LaserRadius > 0 ? LaserRadius : 2.0f);
		if (bHit)
		{
			DrawDebugSphere(MeshComp->GetWorld(), ActualEnd, 20.0f, 8, FColor::Red, false, 0.1f);
		}
		// 목표 높이 시각화
		DrawDebugLine(MeshComp->GetWorld(),
			FVector(LaserStart.X, LaserStart.Y, TargetLaserHeight),
			FVector(LaserEnd.X, LaserEnd.Y, TargetLaserHeight),
			FColor::Yellow, false, 0.1f);
	}

	// 데미지 타이머 업데이트
	DamageTimer += DeltaTime;

	// 데미지 간격마다 데미지 적용
	if (DamageTimer >= DamageInterval)
	{
		DamageTimer = 0.0f;
		DamagedActorsThisTick.Empty();

		if (bHit)
		{
			AActor* HitActor = HitResult.GetActor();

			if (HitActor && !DamagedActorsThisTick.Contains(HitActor))
			{
				if (AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(HitActor))
				{
					// 틱 데미지 계산 (초당 데미지 * 간격)
					float Damage = DamagePerSecond * DamageInterval;

					UGameplayStatics::ApplyPointDamage(
						Player,
						Damage,
						LaserDirection,
						HitResult,
						OwnerBoss->GetController(),
						OwnerBoss,
						UDamageType::StaticClass()
					);

					DamagedActorsThisTick.Add(HitActor);

					PRINT_LOG(TEXT("Laser Hit! Damage: %f"), Damage);
				}
			}
		}
	}
}

void UAZLaserNotifyState::UpdateBeamEffect(const FVector& Start, const FVector& End)
{
	// 방식 1: 나이아가라 파라미터 업데이트
	if (BeamComponent)
	{
		BeamComponent->SetVectorParameter(BeamStartParamName, Start);
		BeamComponent->SetVectorParameter(BeamEndParamName, End);
	}

	// 방식 2: 스플라인 포인트 업데이트
	if (BeamSpline)
	{
		BeamSpline->SetWorldLocationAtSplinePoint(0, Start);
		BeamSpline->SetWorldLocationAtSplinePoint(1, End);
		BeamSpline->UpdateSpline();
	}

	// 충돌 지점 이펙트 업데이트
	if (ImpactEffect && !ImpactComponent)
	{
		ImpactComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			OwnerBoss->GetWorld(),
			ImpactEffect,
			End,
			FRotator::ZeroRotator,
			FVector(1.0f),
			false,
			false
		);
	}

	if (ImpactComponent)
	{
		ImpactComponent->SetWorldLocation(End);
	}
}