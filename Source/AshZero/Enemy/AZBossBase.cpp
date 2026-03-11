// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/AZBossBase.h"
#include "Enemy/AZBossAIController.h"
#include "Enemy/AZEnemyFSM.h"
#include "Enemy/UI/AZEnemyHealthUIComponent.h"
#include "Components/AZHealthComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "BrainComponent.h"
#include "Character/AZPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "AshZero.h"

AAZBossBase::AAZBossBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AAZBossAIController::StaticClass();
	CurrentStamina = MaxStamina;
}

void AAZBossBase::BeginPlay()
{
	Super::BeginPlay();

	if (FSMComponent)
	{
		FSMComponent->SetComponentTickEnabled(false);
		FSMComponent->SetActive(false);
	}

	CurrentStamina = MaxStamina;

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AAZBossBase::OnBossTakeAnyDamage);
		UpdateBlackboardValues();
	}
}

void AAZBossBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // 부모 Tick 호출

	if (!HasAuthority()) return;
	if (bIsInPhaseTransition) return;

	// 타겟 유효성 검사
	if (CurrentAggroController && !IsValidTarget(CurrentAggroController))
	{
		UpdateAggroTarget();
	}

	RegenStamina(DeltaTime);
	UpdateBlackboardValues();

	if (!bIsPerformingPattern)
	{
		AAZBossAIController* BossAI = GetBossAIController();
		if (BossAI && BossAI->GetCurrentBehavior() == EBossBehavior::Turn)
		{
			RotateToTargetWithTurn(DeltaTime);
		}
	}
}

void AAZBossBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAZBossBase, CurrentPhase);
	DOREPLIFETIME(AAZBossBase, CurrentStamina);
}

void AAZBossBase::OnPlayerDetected()
{
	// 보스 UI 활성화
	if (HealthUIComponent && !bBossUIActivated)
	{
		HealthUIComponent->ActivateBossUI();
		bBossUIActivated = true;
		PRINT_LOG(TEXT("Boss UI Activated!"));
	}

	// 어그로 시스템 시작
	if (HasAuthority() && !CurrentAggroController)
	{
		UpdateAggroTarget();
	}
}

void AAZBossBase::OnRep_CurrentPhase()
{
	OnPhaseChanged(CurrentPhase);
	SetPhaseIndex(CurrentPhase);
}

void AAZBossBase::CheckPhaseTransition()
{
	if (!HasAuthority() || bIsInPhaseTransition) return;

	float HealthPercent = GetHealthPercent();

	for (int32 i = CurrentPhase; i < PhaseSettings.Num(); i++)
	{
		if (HealthPercent <= PhaseSettings[i].HealthThreshold)
		{
			bIsInPhaseTransition = true;
			CurrentPhase = i + 1;
			SetPhaseIndex(CurrentPhase);

			if (HealthComponent)
			{
				HealthComponent->SetInvincible(true);
			}

			//  진행 중인 패턴 강제 종료 
			if (bIsPerformingPattern)
			{
				bIsPerformingPattern = false;
				SelectedAttackIndex = -1;
			}

			//  AI 행동 중지 
			if (AAZBossAIController* BossAI = GetBossAIController())
			{
				BossAI->StopMovement();

				if (UBrainComponent* Brain = BossAI->GetBrainComponent())
				{
					Brain->PauseLogic(TEXT("Phase Transition"));
				}
			}

			//  이동 속도 0으로 
			if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
			{
				MoveComp->StopMovementImmediately();
			}

			MulticastPhaseTransition(CurrentPhase);

			GetWorld()->GetTimerManager().SetTimer(
				PhaseTransitionTimerHandle,
				this,
				&AAZBossBase::EndPhaseTransition,
				PhaseTransitionInvincibleTime,
				false
			);

			PRINT_LOG(TEXT("Boss Phase Transition: %d (Health: %.1f%%)"), CurrentPhase, HealthPercent * 100.f);
			break;
		}
	}
}

void AAZBossBase::OnPhaseChanged_Implementation(int32 NewPhase)
{
	// 페이즈 전환 몽타주 재생
	if (PhaseTransitionMontage)
	{
		MulticastPlayMontage(PhaseTransitionMontage);
	}

	OnPhaseChangedDelegate.Broadcast(NewPhase);
}

void AAZBossBase::MulticastPhaseTransition_Implementation(int32 NewPhase)
{
	OnPhaseChanged(NewPhase);
}

void AAZBossBase::EndPhaseTransition()
{
	bIsInPhaseTransition = false;
	if (HealthComponent)
	{
		HealthComponent->SetInvincible(false);
	}

	if (AAZBossAIController* BossAI = GetBossAIController())
	{
		if (UBrainComponent* Brain = BossAI->GetBrainComponent())
		{
			Brain->ResumeLogic(TEXT("Phase Transition End"));
		}
		BossAI->SetBehavior(EBossBehavior::Idle);
	}
}


// ===== 페이즈 헬퍼 함수 =====

float AAZBossBase::GetCurrentAttackThreshold() const
{
	int32 PhaseIndex = CurrentPhase - 1;
	if (PhaseSettings.IsValidIndex(PhaseIndex))
	{
		return PhaseSettings[PhaseIndex].AttackThreshold;
	}
	return 35.0f;
}

float AAZBossBase::GetCurrentStrafeExitThreshold() const
{
	int32 PhaseIndex = CurrentPhase - 1;
	if (PhaseSettings.IsValidIndex(PhaseIndex))
	{
		return PhaseSettings[PhaseIndex].StrafeExitThreshold;
	}
	return 40.0f;
}

float AAZBossBase::GetCurrentApproachSpeed() const
{
	int32 PhaseIndex = CurrentPhase - 1;
	if (PhaseSettings.IsValidIndex(PhaseIndex))
	{
		return PhaseSettings[PhaseIndex].ApproachSpeed;
	}
	return 300.0f;
}

float AAZBossBase::GetCurrentStrafeSpeed() const
{
	int32 PhaseIndex = CurrentPhase - 1;
	if (PhaseSettings.IsValidIndex(PhaseIndex))
	{
		return PhaseSettings[PhaseIndex].StrafeSpeed;
	}
	return 250.0f;
}

// ===== 스태미나 시스템 =====
void AAZBossBase::ConsumeStamina(float Amount)
{
	if (!HasAuthority()) return;
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);
}

bool AAZBossBase::HasEnoughStamina(float Amount) const
{
	return CurrentStamina >= Amount;
}

float AAZBossBase::GetStaminaPercent() const
{
	return MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f;
}

void AAZBossBase::RegenStamina(float DeltaTime)
{
	if (!bIsPerformingPattern && CurrentStamina < MaxStamina)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRegenRate * DeltaTime, 0.0f, MaxStamina);
	}
}

// ===== 공격 선택 시스템 =====

int32 AAZBossBase::SelectAttackIndex()
{
	int32 PhaseIndex = CurrentPhase - 1;
	if (!PhaseSettings.IsValidIndex(PhaseIndex))
	{
		SelectedAttackIndex = 0;
		return 0;
	}

	const TArray<int32>& AvailableIndices = PhaseSettings[PhaseIndex].AvailablePatternIndices;

	if (AvailableIndices.Num() == 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("SelectAttackIndex: No available patterns for phase %d"), CurrentPhase);
		SelectedAttackIndex = 0;
		return 0;
	}

	for (int32 i = AvailableIndices.Num() - 1; i >= 0; i--)
	{
		int32 PatternIndex = AvailableIndices[i];

		if (!AttackPatterns.IsValidIndex(PatternIndex))
		{
			continue;
		}

		const FBossAttackData& AttackData = AttackPatterns[PatternIndex];

		// 연속패턴 방지
		if (!AttackData.bCanRepeat && PatternIndex == LastAttackIndex)
		{
			continue;
		}

		float Roll = FMath::RandRange(0.0f, 100.0f);

		if (Roll <= AttackData.Chance)
		{
			SelectedAttackIndex = PatternIndex;
			return PatternIndex;
		}
	}

	// Fallback: 배열의 첫 번째 패턴 사용
	int32 FallbackIndex = AvailableIndices[0];
	if (AttackPatterns.IsValidIndex(FallbackIndex))
	{
		SelectedAttackIndex = FallbackIndex;
		return FallbackIndex;
	}

	SelectedAttackIndex = 0;
	return 0;
}

bool AAZBossBase::PerformAttackByIndex(int32 AttackIndex)
{
	if (!HasAuthority()) return false;
	if (bIsPerformingPattern) return false;
	if (!AttackPatterns.IsValidIndex(AttackIndex)) return false;

	const FBossAttackData& AttackData = AttackPatterns[AttackIndex];

	if (!AttackData.Montage)
	{
		return false;
	}

	ConsumeStamina(AttackData.StaminaCost);

	bIsPerformingPattern = true;

	MulticastPlayMontage(AttackData.Montage);

	// 몽타주 종료 델리게이트
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AAZBossBase::OnPatternMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackData.Montage);
	}

	return true;
}

float AAZBossBase::GetCurrentAttackDamage() const
{
	if (AttackPatterns.IsValidIndex(SelectedAttackIndex))
	{
		return AttackPatterns[SelectedAttackIndex].Damage;
	}
	return 10.0f;
}

float AAZBossBase::GetAttackRange(int32 AttackIndex) const
{
	if (AttackPatterns.IsValidIndex(AttackIndex))
	{
		return AttackPatterns[AttackIndex].AttackRange;
	}
	return 300.0f;
}

bool AAZBossBase::DoesAttackNeedApproach(int32 AttackIndex) const
{
	if (AttackPatterns.IsValidIndex(AttackIndex))
	{
		return AttackPatterns[AttackIndex].bNeedApproach;
	}
	return true;
}

void AAZBossBase::OnPatternFinished()
{
	bIsPerformingPattern = false;
	SelectedAttackIndex = -1;

	if (AAZBossAIController* BossAI = GetBossAIController())
	{
		BossAI->SetBehavior(EBossBehavior::Idle);
	}

	OnPatternFinishedDelegate.Broadcast();
}

void AAZBossBase::OnPatternMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnPatternFinished();
}

// ===== 타겟 관련 =====
float AAZBossBase::GetDistanceToTarget() const
{
	AActor* Target = GetCurrentTarget();
	if (!Target) return MAX_FLT;

	return FVector::Dist(GetActorLocation(), Target->GetActorLocation());
}

AActor* AAZBossBase::GetCurrentTarget() const
{
	if (AAZBossAIController* BossAI = GetBossAIController())
	{
		return BossAI->GetBlackboardTarget();
	}
	return nullptr;
}

void AAZBossBase::StartAggroTimer()
{
	GetWorldTimerManager().SetTimer(
		AggroUpdateTimerHandle,
		this,
		&AAZBossBase::UpdateAggroTarget,
		AggroUpdateInterval,
		true  // 반복
	);
}

void AAZBossBase::StopAggroTimer()
{
	GetWorldTimerManager().ClearTimer(AggroUpdateTimerHandle);
}

void AAZBossBase::UpdateAggroTarget()
{
	if (!HasAuthority()) return;

	AController* BestTarget = nullptr;
	float MaxDamage = -1.0f;

	// 1. 가장 데미지를 많이 넣은 "살아있는" 플레이어 찾기
	for (auto& Pair : DamageHistory)
	{
		if (IsValidTarget(Pair.Key))
		{
			if (Pair.Value > MaxDamage)
			{
				MaxDamage = Pair.Value;
				BestTarget = Pair.Key;
			}
		}
	}

	// 2. 타겟 설정
	if (BestTarget)
	{
		CurrentAggroController = BestTarget;

		if (AAZBossAIController* BossAI = GetBossAIController())
		{
			BossAI->SetBlackboardTarget(BestTarget->GetPawn());
		}

		PRINT_LOG(TEXT("Aggro Target: %s (Damage: %.1f)"), *BestTarget->GetName(), MaxDamage);
	}
	else
	{
		// 모든 기록된 플레이어가 죽음 → 아무나 찾기
		AController* FallbackTarget = FindAnyAlivePlayer();
		if (FallbackTarget)
		{
			CurrentAggroController = FallbackTarget;

			if (AAZBossAIController* BossAI = GetBossAIController())
			{
				BossAI->SetBlackboardTarget(FallbackTarget->GetPawn());
			}

			PRINT_LOG(TEXT("Aggro Fallback: %s"), *FallbackTarget->GetName());
		}
	}
	// 3. 데미지 기록 초기화
	DamageHistory.Empty();
}

bool AAZBossBase::IsValidTarget(AController* InController) const
{
	if (!InController) return false;

	APawn* Pawn = InController->GetPawn();
	if (!Pawn) return false;

	// 플레이어 캐릭터면 사망 체크
	if (AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(Pawn))
	{
		return !Player->bIsDead;
	}

	return true;
}

AController* AAZBossBase::FindAnyAlivePlayer() const
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AController* PC = It->Get();
		if (IsValidTarget(PC))
		{
			return PC;
		}
	}
	return nullptr;
}


void AAZBossBase::RotateToTargetWithTurn(float DeltaTime)
{
	AActor* Target = GetCurrentTarget();
	if (!Target) return;

	FVector DirectionToTarget = Target->GetActorLocation() - GetActorLocation();
	DirectionToTarget.Z = 0.0f;
	if (DirectionToTarget.IsNearlyZero()) return;

	FRotator TargetRotation = DirectionToTarget.Rotation();
	FRotator CurrentRotation = GetActorRotation();

	// TurnDirection 계산 (-1 = 왼쪽, 0 = 정면, 1 = 오른쪽)
	float YawDiff = FRotator::NormalizeAxis(TargetRotation.Yaw - CurrentRotation.Yaw);
	TurnDirection = FMath::Clamp(YawDiff / 90.0f, -1.0f, 1.0f);

	// 회전
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, TurnSpeed);
	SetActorRotation(NewRotation);
}

// ===== 사망 =====
void AAZBossBase::OnDeath(FVector DeathLocation, AActor* Killer)
{
	if (!HasAuthority()) return;

	StopAggroTimer();
	DamageHistory.Empty();
	CurrentAggroController = nullptr;

	// 몽타주 중지
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.1f);
	}

	// 패턴 상태 초기화
	bIsPerformingPattern = false;
	SelectedAttackIndex = -1;

	// AI 완전 중지
	if (AAZBossAIController* BossAI = GetBossAIController())
	{
		BossAI->StopMovement();

		if (UBrainComponent* Brain = BossAI->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Boss Dead"));
		}
	}

	// 포탈 스폰
	if (PortalClass)
	{
		float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		FVector SpawnLocation = GetActorLocation() - FVector(0.f, 0.f, HalfHeight) + PortalSpawnOffset;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(PortalClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	}

	// 부모 OnDeath 호출
	Super::OnDeath(DeathLocation, Killer);

	PRINT_LOG(TEXT("Boss Dead!"));
}

// ===== 데미지 & 유틸리티 =====

void AAZBossBase::OnBossTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsInPhaseTransition) return;

	if (InstigatedBy && InstigatedBy->IsPlayerController())
	{
		float& TotalDamage = DamageHistory.FindOrAdd(InstigatedBy);
		TotalDamage += Damage;

		if (!CurrentAggroController)
		{
			CurrentAggroController = InstigatedBy;

			if (AAZBossAIController* BossAI = GetBossAIController())
			{
				BossAI->SetBlackboardTarget(InstigatedBy->GetPawn());
			}

			StartAggroTimer();
		}
		OnPlayerDetected();
	}

	CheckPhaseTransition();
}

AAZBossAIController* AAZBossBase::GetBossAIController() const
{
	return Cast<AAZBossAIController>(GetController());
}

void AAZBossBase::UpdateBlackboardValues()
{
	AAZBossAIController* BossAI = GetBossAIController();
	if (!BossAI) return;

	UBlackboardComponent* BB = BossAI->GetBlackboardComponent();
	if (!BB) return;

	BB->SetValueAsInt(TEXT("CurrentPhase"), CurrentPhase);
	BB->SetValueAsFloat(TEXT("Stamina"), CurrentStamina);
	BB->SetValueAsFloat(TEXT("DistanceToTarget"), GetDistanceToTarget());
	BB->SetValueAsBool(TEXT("IsPerformingPattern"), bIsPerformingPattern);
	BB->SetValueAsBool(TEXT("IsInPhaseTransition"), bIsInPhaseTransition);
	BB->SetValueAsInt(TEXT("SelectedAttackIndex"), SelectedAttackIndex);
	BB->SetValueAsFloat(TEXT("TurnDirection"), TurnDirection);
}

float AAZBossBase::GetHealthPercent() const
{
	if (HealthComponent)
	{
		return HealthComponent->GetHealthPercent();
	}
	return 1.0f;
}