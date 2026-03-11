// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AZEnemyFSM.h"
#include "Enemy/AZEnemyBase.h"
#include "Enemy/AZAIController.h"
#include "Components/AZHealthComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Character/AZPlayerCharacter.h"
#include "AshZero.h"

// Sets default values for this component's properties
UAZEnemyFSM::UAZEnemyFSM()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UAZEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<AAZEnemyBase>(GetOwner());
	if (Owner)
	{
		AIController = Cast<AAIController>(Owner->GetController());
        PatrolCenter = Owner->GetActorLocation();
	}
}

void UAZEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_Authority)
	{
		UpdateStateMachine(DeltaTime);
	}
}

void UAZEnemyFSM::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAZEnemyFSM, State);
    DOREPLIFETIME(UAZEnemyFSM, CurrentTarget);
}

void UAZEnemyFSM::UpdateStateMachine(float DeltaTime)
{
    switch (State)
    {
    case EEnemyState::Idle:
        IdleState(DeltaTime);
        break;
    case EEnemyState::Patrol:
        PatrolState(DeltaTime);
        break;
    case EEnemyState::Chase:
        ChaseState(DeltaTime);
        break;
    case EEnemyState::Attack:
        AttackState(DeltaTime);
        break;
    case EEnemyState::Damage:
        DamageState(DeltaTime);
        break;
    case EEnemyState::Die:
        DieState(DeltaTime);
        break;
    }
}

void UAZEnemyFSM::IdleState(float DeltaTime)
{
	StateTimer += DeltaTime;

	// 범위 내 플레이어 체크
	if (AAZPlayerCharacter* Player = FindPlayerInRange(DetectionRange))
	{
		CurrentTarget = Player;
		ServerSetState(EEnemyState::Chase);
		return;
	}

	// 대기 시간 후 순찰 시작
	if (StateTimer >= PatrolWaitTime)
	{
		if (GetRandomPatrolLocation(PatrolLocation))
		{
			ServerSetState(EEnemyState::Patrol);
		}
		StateTimer = 0.0f;
	}
}

void UAZEnemyFSM::PatrolState(float DeltaTime)
{
	if (!AIController || !Owner) return;

	// 범위 내 플레이어 체크 (우선순위 높음)
	if (AAZPlayerCharacter* Player = FindPlayerInRange(DetectionRange))
	{
		CurrentTarget = Player;
		AIController->StopMovement();
		ServerSetState(EEnemyState::Chase);
		return;
	}

	// 순찰 이동
	Owner->SetMovementSpeed(PatrolSpeed);

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(PatrolLocation);
	MoveRequest.SetAcceptanceRadius(50.0f);

	FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest);

	// 목적지 도착
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		ServerSetState(EEnemyState::Idle);
	}
}

void UAZEnemyFSM::ChaseState(float DeltaTime)
{
	if (!AIController || !Owner) return;

	// 타겟 유효성 체크
	if (!CurrentTarget || CurrentTarget->bIsDead)
	{
		CurrentTarget = nullptr;
		AIController->StopMovement();

		// 현재 위치를 새 순찰 중심으로 설정
		PatrolCenter = Owner->GetActorLocation();
		ServerSetState(EEnemyState::Idle);
		return;
	}

	// 추적 포기 거리 체크
	float Distance = GetDistanceToTarget();
	if (Distance > LoseTargetRange)
	{
		CurrentTarget = nullptr;
		AIController->StopMovement();

		// 현재 위치를 새 순찰 중심으로 설정
		PatrolCenter = Owner->GetActorLocation();
		ServerSetState(EEnemyState::Idle);
		return;
	}

	// 공격 범위 체크
	if (CanAttackTarget())
	{
		AIController->StopMovement();
		ServerSetState(EEnemyState::Attack);
		return;
	}

	// 추적 이동
	Owner->SetMovementSpeed(ChaseSpeed);

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(CurrentTarget);
	MoveRequest.SetAcceptanceRadius(10.0f);
	AIController->MoveTo(MoveRequest);

	// 타겟 방향으로 회전
	FVector Direction = CurrentTarget->GetActorLocation() - Owner->GetActorLocation();
	Direction.Z = 0;
	if (!Direction.IsNearlyZero())
	{
		FRotator TargetRot = Direction.Rotation();
		Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), TargetRot, DeltaTime, 10.0f));
	}
}

void UAZEnemyFSM::AttackState(float DeltaTime)
{
	if (!Owner) return;

	// 타겟 없으면 Idle로
	if (!CurrentTarget || CurrentTarget->bIsDead)
	{
		if (!bIsAttacking)
		{
			CurrentTarget = nullptr;
			ServerSetState(EEnemyState::Idle);
		}
		return;
	}

	// 공격 중이 아닐 때 타겟 방향으로 회전
	if (!bIsAttacking)
	{
		FVector Direction = CurrentTarget->GetActorLocation() - Owner->GetActorLocation();
		Direction.Z = 0;
		if (!Direction.IsNearlyZero())
		{
			FRotator TargetRot = Direction.Rotation();
			Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), TargetRot, DeltaTime, 10.0f));
		}
	}

	AttackTimer += DeltaTime;

	// 공격 실행
	if (AttackTimer >= AttackCooldown && !bIsAttacking)
	{
		AttackTimer = 0.0f;

		if (Owner->AttackMontage)
		{
			bIsAttacking = true;
			Owner->MulticastPlayMontage(Owner->AttackMontage);

			// 몽타주 종료 감지
			if (UAnimInstance* Anim = Owner->GetMesh()->GetAnimInstance())
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &UAZEnemyFSM::OnAttackMontageEnded);
				Anim->Montage_SetEndDelegate(EndDelegate, Owner->AttackMontage);
			}
		}
	}

	// 범위 벗어남 체크 (공격 중 아닐 때만)
	if (!bIsAttacking && !CanAttackTarget())
	{
		ServerSetState(EEnemyState::Chase);
	}
}

void UAZEnemyFSM::DamageState(float DeltaTime)
{
	if (AIController)
	{
		AIController->StopMovement();
	}

	StateTimer += DeltaTime;

	if (StateTimer >= DamageStunTime)
	{
		StateTimer = 0.0f;

		if (CurrentTarget && !CurrentTarget->bIsDead)
		{
			ServerSetState(EEnemyState::Chase);
		}
		else
		{
			CurrentTarget = nullptr;
			ServerSetState(EEnemyState::Idle);
		}
	}
}

void UAZEnemyFSM::DieState(float DeltaTime)
{
	// 사망 상태 - 처리는 Owner에서
}

// ===== 외부 인터페이스 =====

void UAZEnemyFSM::OnDamageReceived(float DamageAmount, AActor* DamageCauser)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (State == EEnemyState::Die) return;

	// 넉다운 체크를 먼저! (Chase 전환보다 우선)
	if (!bHasPlayedKnockdown && Owner)
	{
		UAZHealthComponent* HealthComp = Owner->GetHealthComponent();
		if (HealthComp && HealthComp->GetHealthPercent() <= DamageThresholdPercent)
		{
			bHasPlayedKnockdown = true;

			// 타겟 설정 (상태 전환 없이 직접 설정)
			if (DamageCauser && !CurrentTarget)
			{
				if (AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(DamageCauser))
				{
					CurrentTarget = Player;
				}
			}

			// AI 이동 즉시 중지
			if (AIController)
			{
				AIController->StopMovement();
			}

			ServerSetState(EEnemyState::Damage);
			StateTimer = 0.0f;

			if (Owner->HitReactMontage)
			{
				Owner->MulticastPlayMontage(Owner->HitReactMontage);
			}
			return;  // 여기서 리턴!
		}
	}


	// 공격자를 타겟으로 설정
	if (DamageCauser && !CurrentTarget)
	{
		if (AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(DamageCauser))
		{
			SetTarget(Player);

			// 바로 Chase 상태로 전환
			ServerSetState(EEnemyState::Chase);
		}
	}


	if (!bHasPlayedKnockdown && Owner)
	{
		UAZHealthComponent* HealthComp = Owner->GetHealthComponent();
		if (HealthComp && HealthComp->GetHealthPercent() <= DamageThresholdPercent)
		{

			bHasPlayedKnockdown = true;
			ServerSetState(EEnemyState::Damage);
			StateTimer = 0.0f;

			// 피격 몽타주 재생
			if (Owner->HitReactMontage)
			{
				Owner->MulticastPlayMontage(Owner->HitReactMontage);
			}
		}
	}
}

void UAZEnemyFSM::SetTarget(AAZPlayerCharacter* NewTarget)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	CurrentTarget = NewTarget;

	// Idle/Patrol 상태면 즉시 추적 시작
	if (NewTarget && (State == EEnemyState::Idle || State == EEnemyState::Patrol))
	{
		if (AIController)
		{
			AIController->StopMovement();
		}
		ServerSetState(EEnemyState::Chase);
	}
}

bool UAZEnemyFSM::CanAttackTarget() const
{
	if (!CurrentTarget || !Owner) return false;

	FVector ToTarget = CurrentTarget->GetActorLocation() - Owner->GetActorLocation();
	float Distance = ToTarget.Size();

	// 거리 체크
	if (Distance > AttackRange) return false;

	// 각도 체크
	ToTarget.Normalize();
	float DotProduct = FVector::DotProduct(Owner->GetActorForwardVector(), ToTarget);
	float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	return Angle <= AttackAngle;
}

// ===== 유틸리티 =====

AAZPlayerCharacter* UAZEnemyFSM::FindPlayerInRange(float Range) const
{
	if (!Owner) return nullptr;

	// 모든 플레이어 순회
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(PC->GetPawn());
		if (!Player || Player->bIsDead) continue;

		float Distance = FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());
		if (Distance <= Range)
		{
			return Player;
		}
	}

	return nullptr;
}

bool UAZEnemyFSM::GetRandomPatrolLocation(FVector& OutLocation)
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	if (!NavSystem) return false;

	FNavLocation NavLocation;
	bool bSuccess = NavSystem->GetRandomReachablePointInRadius(
		PatrolCenter,  // 순찰 중심점 기준
		PatrolRadius,
		NavLocation
	);

	if (bSuccess)
	{
		OutLocation = NavLocation.Location;
	}

	return bSuccess;
}

float UAZEnemyFSM::GetDistanceToTarget() const
{
	if (!CurrentTarget || !Owner) return MAX_FLT;
	return FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());
}

void UAZEnemyFSM::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;

	if (CurrentTarget && !CanAttackTarget())
	{
		ServerSetState(EEnemyState::Chase);
	}
}

// ===== 네트워크 =====

void UAZEnemyFSM::ServerSetState_Implementation(EEnemyState NewState)
{
	if (State != NewState)
	{
		State = NewState;
		StateTimer = 0.0f;

		// Attack 상태 진입 시 즉시 공격
		if (NewState == EEnemyState::Attack)
		{
			AttackTimer = AttackCooldown;
		}

		MulticastStateChanged(NewState);
	}
}

void UAZEnemyFSM::MulticastStateChanged_Implementation(EEnemyState NewState)
{
    // 현재는 사용 안 함
    // 상태 진입 시 특수 효과 필요하면 여기에 추가
}

AActor* UAZEnemyFSM::GetTargetActor() const
{
	return CurrentTarget;
}