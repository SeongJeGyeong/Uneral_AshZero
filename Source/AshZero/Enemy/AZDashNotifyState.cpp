// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AZDashNotifyState.h"
#include "Enemy/AZEnemyBase.h"
#include "Enemy/AZEnemyFSM.h"
#include "GameFramework/CharacterMovementComponent.h"

UAZDashNotifyState::UAZDashNotifyState()
{
	DashSpeed = 1500.0f;
	bFixDirectionAtStart = true;
	TrackingRotationSpeed = 5.0f;
}

void UAZDashNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AAZEnemyBase* Enemy = Cast<AAZEnemyBase>(MeshComp->GetOwner());
	if (!Enemy) return;
	if (Enemy->bIsDead) return;

	AActor* Target = Enemy->GetCurrentTarget();

	FVector TargetDir;
	if (Target)
	{
		TargetDir = (Target->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal2D();
	}
	else
	{
		TargetDir = Enemy->GetActorForwardVector();
	}

	UpdateDashDirection(TargetDir);

	if (MovementType == EDashMovementType::SpeedBased)
	{
		if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
		{
			OriginalMaxWalkSpeed = Movement->MaxWalkSpeed;
			Movement->MaxWalkSpeed = DashSpeed;
		}
	}
}

void UAZDashNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	AAZEnemyBase* Enemy = Cast<AAZEnemyBase>(MeshComp->GetOwner());
	if (!Enemy) return;

	// 추적 모드: 방향 갱신
	if (!bFixDirectionAtStart)
	{
		AActor* Target = Enemy->GetCurrentTarget();
		if (Target)
		{
			FVector CurrentToTargetDir = (Target->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal2D();

			if (!bFixDirectionAtStart)
			{
				FRotator CurrentRot = Enemy->GetActorRotation();
				FRotator TargetRot = CurrentToTargetDir.Rotation();

				// RInterpTo를 사용하여 TrackingRotationSpeed 속도로 부드럽게 회전
				Enemy->SetActorRotation(FMath::RInterpTo(CurrentRot, TargetRot, FrameDeltaTime, TrackingRotationSpeed));

				// 실시간 추적 모드이므로 대시 방향도 타겟 위치 변화에 따라 계속 갱신
				UpdateDashDirection(CurrentToTargetDir);
			}
		}
	}

	// 이동 처리
	if (MovementType == EDashMovementType::SpeedBased)
	{
		// 방식 1: AddMovementInput (가속/감속 로직 거침)
		Enemy->AddMovementInput(DashDirection, 1.0f);
	}
	else
	{
		// 방식 2: Velocity 직접 설정 (즉시 해당 속도)
		if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
		{
			Movement->Velocity = DashDirection * DashSpeed;
		}
	}
}

void UAZDashNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AAZEnemyBase* Enemy = Cast<AAZEnemyBase>(MeshComp->GetOwner());
	if (!Enemy) return;

	UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement();
	if (!Movement) return;

	if (MovementType == EDashMovementType::SpeedBased)
	{
		// 방식 1: 원래 속도 복구
		Movement->MaxWalkSpeed = OriginalMaxWalkSpeed;
	}

	// 둘 다 이동 정지
	Movement->StopMovementImmediately();

}

void UAZDashNotifyState::UpdateDashDirection(const FVector& LookDir)
{
	switch (DashDirectionType)
	{
	case EDashDirection::Forward:  DashDirection = LookDir; break;
	case EDashDirection::Backward: DashDirection = -LookDir; break;
	case EDashDirection::Left:     DashDirection = LookDir.RotateAngleAxis(-90.0f, FVector::UpVector); break;
	case EDashDirection::Right:    DashDirection = LookDir.RotateAngleAxis(90.0f, FVector::UpVector); break;
	}
}