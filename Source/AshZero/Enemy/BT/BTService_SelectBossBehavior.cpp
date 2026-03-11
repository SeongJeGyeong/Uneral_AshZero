// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_SelectBossBehavior.h"
#include "Enemy/AZBossBase.h"
#include "Enemy/AZBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/AZBossData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"

UBTService_SelectBossBehavior::UBTService_SelectBossBehavior()
{
	NodeName = TEXT("Select Boss Behavior");
	Interval = 0.1f;
	RandomDeviation = 0.0f;

	// 기본값 설정
	TurnAngleThreshold = 30.0f;

	// 블랙보드 키 필터 설정
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SelectBossBehavior, TargetKey), AActor::StaticClass());
	StaminaKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SelectBossBehavior, StaminaKey));
	DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SelectBossBehavior, DistanceToTargetKey));
	BehaviorKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SelectBossBehavior, BehaviorKey), StaticEnum<EBossBehavior>());
}

void UBTService_SelectBossBehavior::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	AAZBossBase* Boss = Cast<AAZBossBase>(AIController->GetPawn());
	if (!Boss) return;

	// 패턴 수행 중이면 Behavior 변경 안 함
	if (Boss->bIsPerformingPattern)
	{
		return;
	}

	// 타겟 확인
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));

	// 타겟 없으면 플레이어 자동 탐색
	if (!Target)
	{
		APlayerController* PC = Boss->GetWorld()->GetFirstPlayerController();
		if (PC && PC->GetPawn())
		{
			Target = PC->GetPawn();
			BB->SetValueAsObject(TargetKey.SelectedKeyName, Target);
		}
		else
		{
			SetBehaviorWithSpeed(Boss, BB, EBossBehavior::Idle);
			return;
		}
	}

	// 거리 계산 + 블랙보드 업데이트
	float Distance = FVector::Dist(Boss->GetActorLocation(), Target->GetActorLocation());
	BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);

	// 스태미나 업데이트
	float Stamina = Boss->CurrentStamina;
	BB->SetValueAsFloat(StaminaKey.SelectedKeyName, Stamina);

	// 페이즈별 임계값
	float AttackThreshold = Boss->GetCurrentAttackThreshold();
	float StrafeExitThreshold = Boss->GetCurrentStrafeExitThreshold();

	// ========== 각도 체크 ==========
	FVector DirectionToTarget = Target->GetActorLocation() - Boss->GetActorLocation();
	DirectionToTarget.Z = 0.0f;
	DirectionToTarget.Normalize();

	FVector BossForward = Boss->GetActorForwardVector();
	BossForward.Z = 0.0f;
	BossForward.Normalize();

	float DotProduct = FVector::DotProduct(BossForward, DirectionToTarget);
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	// ========== 행동 결정 로직 ==========
	EBossBehavior NewBehavior = EBossBehavior::Idle;

	// 1. 각도 > TurnAngleThreshold → Turn
	if (AngleDegrees > TurnAngleThreshold)
	{
		NewBehavior = EBossBehavior::Turn;
	}
	// 2. 현재 Strafe 중인 경우
	else if (bIsCurrentlyStrafing)
	{
		if (Stamina >= StrafeExitThreshold)
		{
			// Strafe 종료 → Attack
			bIsCurrentlyStrafing = false;
			NewBehavior = EBossBehavior::Attack;
			UE_LOG(LogTemp, Warning, TEXT("Boss - Strafe EXIT (Stamina: %.1f >= %.1f)"), Stamina, StrafeExitThreshold);
		}
		else
		{
			// 스태미나 부족 → Strafe 유지
			NewBehavior = EBossBehavior::Strafe;
		}
	}
	// 3. Strafe 중이 아닌 경우
	else
	{
		// 스태미나 부족 → Strafe 시작
		if (Stamina < AttackThreshold)
		{
			NewBehavior = EBossBehavior::Strafe;
			bIsCurrentlyStrafing = true;
			Boss->SelectedAttackIndex = -1;  // 공격 선택 초기화
			UE_LOG(LogTemp, Warning, TEXT("Boss - Strafe START (Stamina: %.1f < %.1f)"), Stamina, AttackThreshold);
		}
		// 스태미나 충분 → Attack (공격 선택 및 이동은 Task에서)
		else
		{
			NewBehavior = EBossBehavior::Attack;
		}
	}

	EBossBehavior CurrentBehavior =
		static_cast<EBossBehavior>(BB->GetValueAsEnum(BehaviorKey.SelectedKeyName));

	if (CurrentBehavior != NewBehavior)
	{
		SetBehaviorWithSpeed(Boss, BB, NewBehavior);
	}
	// 디버그 로그
	UE_LOG(LogTemp, Log, TEXT("Boss - Phase:%d, Behavior:%d, Dist:%.1f, Stamina:%.1f"),
		Boss->CurrentPhase,
		static_cast<int32>(NewBehavior),
		Distance,
		Stamina);
}

void UBTService_SelectBossBehavior::SetBehaviorWithSpeed(AAZBossBase* Boss, UBlackboardComponent* BB, EBossBehavior NewBehavior)
{
	// 이동 속도 설정
	if (UCharacterMovementComponent* MovementComp = Boss->GetCharacterMovement())
	{
		switch (NewBehavior)
		{
		case EBossBehavior::Idle:
		case EBossBehavior::Turn:
			MovementComp->MaxWalkSpeed = Boss->IdleSpeed;
			break;
		case EBossBehavior::Strafe:
			MovementComp->MaxWalkSpeed = Boss->GetCurrentStrafeSpeed();
			break;
		case EBossBehavior::Attack:
			MovementComp->MaxWalkSpeed = Boss->GetCurrentApproachSpeed();
			break;
		default:
			break;
		}
	}

	// Behavior 업데이트
	BB->SetValueAsEnum(BehaviorKey.SelectedKeyName, static_cast<uint8>(NewBehavior));
}

