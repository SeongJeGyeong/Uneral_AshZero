
#include "Enemy/AZAreaAttackNotifyState.h"
#include "Enemy/AZBossBase.h"
#include "Character/AZPlayerCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "AshZero.h"

UAZAreaAttackNotifyState::UAZAreaAttackNotifyState()
{
}

void UAZAreaAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner()) return;

	OwnerBoss = Cast<AAZBossBase>(MeshComp->GetOwner());
	if (!OwnerBoss) return;

	// 1. VFX 스폰 및 부착
	if (AreaVFX)
	{
		SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			AreaVFX,
			MeshComp,
			SocketName,
			FVector::ZeroVector,
            RotationOffset,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	TraceTimer = 0.0f;
}

void UAZAreaAttackNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	PerformAreaTrace(MeshComp, FrameDeltaTime);
}

void UAZAreaAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// 2. 종료 시 VFX 소멸
	if (SpawnedVFX)
	{
		SpawnedVFX->DestroyComponent();
		SpawnedVFX = nullptr;
	}
}
void UAZAreaAttackNotifyState::PerformAreaTrace(USkeletalMeshComponent* MeshComp, float DeltaTime)
{
    if (!OwnerBoss) return;
    if (OwnerBoss->bIsDead) return;
    // 1. 기본 소켓 정보 (총구)
    FVector Origin = MeshComp->GetSocketLocation(SocketName);
    FRotator SocketRotation = MeshComp->GetSocketRotation(SocketName);

    // 2. 공격 방향 계산 (여기가 핵심!)
    FRotator FinalAttackRotation = SocketRotation; // 기본은 총구 방향

    AActor* Target = OwnerBoss->GetCurrentTarget();
    if (bTargetHeightTracking && Target)
    {
        // 타겟을 향하는 "전체" 각도 계산
        FVector TargetLoc = Target->GetActorLocation();
        TargetLoc.Z += TargetHeightOffset;

        FVector ToTarget = TargetLoc - Origin;
        FRotator LookAtRot = ToTarget.Rotation();
        FinalAttackRotation.Pitch = LookAtRot.Pitch;
    }

    // 최종 공격 방향 벡터 (이걸로 데미지도 주고 디버그도 그림)
    FVector AttackDirection = FinalAttackRotation.Vector();


    // 3. 디버그 그리기 (AttackDirection 사용)
    float AngleInRadians = FMath::DegreesToRadians(AttackAngle);
    if (bDebugDraw)
    {
        // 이제 노란 원뿔은 보스 정면을 보되, 위아래로만 까닥거립니다.
        DrawDebugCone(
            OwnerBoss->GetWorld(),
            Origin,
            AttackDirection,
            AttackRange,
            AngleInRadians,
            AngleInRadians,
            12,
            FColor::Yellow,
            false,
            -1.0f, // LifeTime (-1이면 프레임 단위)
            0,
            1.0f
        );
    }

    // 4. VFX 회전 업데이트 (시각적 일치)
    // 공격 판정 방향(FinalAttackRotation)에 + 님께서 설정한 VFX 오프셋(0, 270, 15)을 더해줌
    if (SpawnedVFX)
    {
        SpawnedVFX->SetWorldRotation(FinalAttackRotation + RotationOffset);
    }


    // 5. 데미지 타이머 & 판정 (기존과 동일)
    TraceTimer += DeltaTime;
    if (TraceTimer < DamageInterval) return;
    TraceTimer = 0.0f;

    if (!Target) return;

    FVector ToTarget = Target->GetActorLocation() - Origin;
    float Distance = ToTarget.Size();
    if (Distance > AttackRange) return;

    FVector ToTargetDir = ToTarget.GetSafeNormal();

    // AttackDirection(위에서 만든 하이브리드 벡터)과 비교
    float DotProduct = FVector::DotProduct(AttackDirection, ToTargetDir);

    if (DotProduct >= FMath::Cos(AngleInRadians))
    {
        float BaseDamage = OwnerBoss->GetCurrentAttackDamage();
        UGameplayStatics::ApplyPointDamage(
            Target,
            BaseDamage * DamageInterval,
            AttackDirection,
            FHitResult(),
            OwnerBoss->GetController(),
            OwnerBoss,
            UDamageType::StaticClass()
        );
    }
}