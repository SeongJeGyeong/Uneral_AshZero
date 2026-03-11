// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_TurnToTarget.h"
#include "Enemy/AZBossBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
	NodeName = TEXT("Boss Turn To Target");
	bNotifyTick = true;
	bCreateNodeInstance = true;

	TurnSpeed = 180.0f;
	AcceptanceAngle = 5.0f;
	bUseAnimationTurn = true;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_TurnToTarget, TargetKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	CachedBoss = Cast<AAZBossBase>(AIController->GetPawn());
	if (!CachedBoss)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	// 각도 체크 - 이미 바라보고 있으면 바로 성공
	float AngleToTarget = GetAngleToTarget(CachedBoss, Target);
	if (FMath::Abs(AngleToTarget) <= AcceptanceAngle)
	{
		UE_LOG(LogTemp, Log, TEXT("TurnToTarget: Already facing target (Angle: %.1f)"), AngleToTarget);
		return EBTNodeResult::Succeeded;
	}

	// Turn 애니메이션 재생
	if (bUseAnimationTurn)
	{
		UAnimMontage* TurnMontage = (AngleToTarget > 0) ? TurnRightMontage : TurnLeftMontage;

		if (TurnMontage)
		{
			CachedBoss->PlayAnimMontage(TurnMontage);
			bIsTurning = true;

			UE_LOG(LogTemp, Log, TEXT("TurnToTarget: Playing %s (Angle: %.1f)"), (AngleToTarget > 0) ? TEXT("TurnRight") : TEXT("TurnLeft"), AngleToTarget);
		}
	}

	bIsTurning = true;
	return EBTNodeResult::InProgress;
}

void UBTTask_TurnToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!CachedBoss)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!Target)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 타겟 방향 계산
	FVector DirectionToTarget = Target->GetActorLocation() - CachedBoss->GetActorLocation();
	DirectionToTarget.Z = 0.0f;
	DirectionToTarget.Normalize();

	FRotator TargetRotation = DirectionToTarget.Rotation();
	FRotator CurrentRotation = CachedBoss->GetActorRotation();

	// 부드럽게 회전
	FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaSeconds, TurnSpeed);
	CachedBoss->SetActorRotation(NewRotation);

	// 각도 체크
	float AngleToTarget = GetAngleToTarget(CachedBoss, Target);

	// 회전 완료 체크
	if (FMath::Abs(AngleToTarget) <= AcceptanceAngle)
	{
		UE_LOG(LogTemp, Log, TEXT("TurnToTarget: Turn complete! (Angle: %.1f)"), AngleToTarget);

		// 애니메이션 중지 (필요시)
		if (bUseAnimationTurn && (TurnLeftMontage || TurnRightMontage))
		{
			CachedBoss->StopAnimMontage();
		}

		bIsTurning = false;
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_TurnToTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (CachedBoss && bIsTurning)
	{
		if (bUseAnimationTurn && (TurnLeftMontage || TurnRightMontage))
		{
			CachedBoss->StopAnimMontage();
		}
	}

	bIsTurning = false;
	CachedBoss = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("TurnToTarget: Aborted"));
	return EBTNodeResult::Aborted;
}

float UBTTask_TurnToTarget::GetAngleToTarget(AAZBossBase* Boss, AActor* Target) const
{
	if (!Boss || !Target) return 0.0f;

	FVector DirectionToTarget = Target->GetActorLocation() - Boss->GetActorLocation();
	DirectionToTarget.Z = 0.0f;
	DirectionToTarget.Normalize();

	FVector BossForward = Boss->GetActorForwardVector();
	BossForward.Z = 0.0f;
	BossForward.Normalize();

	FVector BossRight = Boss->GetActorRightVector();
	BossRight.Z = 0.0f;
	BossRight.Normalize();

	// 전방 각도 (0 ~ 180)
	float DotForward = FVector::DotProduct(BossForward, DirectionToTarget);
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotForward, -1.0f, 1.0f)));

	// 좌우 판별 (양수: 오른쪽, 음수: 왼쪽)
	float DotRight = FVector::DotProduct(BossRight, DirectionToTarget);

	return (DotRight >= 0) ? AngleDegrees : -AngleDegrees;
}