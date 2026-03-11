// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AZDashNotifyState.generated.h"

UENUM(BlueprintType)
enum class EDashMovementType : uint8
{
	// MaxWalkSpeed 변경 + AddMovementInput (가속/감속 있음 0 -> 1500 가속도를 주며 가속)
	SpeedBased		UMETA(DisplayName = "Speed Based"),

	// Velocity 직접 설정 (즉시 해당 속도 0 -> 1500 으로 즉시 속도 변경)
	VelocityBased	UMETA(DisplayName = "Velocity Based")
};

UENUM(BlueprintType)
enum class EDashDirection : uint8
{
	Forward		UMETA(DisplayName = "Forward (To Target)"),
	// 타겟 반대 방향으로 후퇴 (타겟 바라보면서)
	Backward	UMETA(DisplayName = "Backward (Away from Target)"),
	// 왼쪽으로 이동 (타겟 바라보면서)
	Left		UMETA(DisplayName = "Left (Strafe)"),
	// 오른쪽으로 이동 (타겟 바라보면서)
	Right		UMETA(DisplayName = "Right (Strafe)")
};

UCLASS()
class ASHZERO_API UAZDashNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	

public:
	UAZDashNotifyState();

	// 이동 방식 선택
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	EDashMovementType MovementType = EDashMovementType::VelocityBased;

	// 돌진 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashSpeed = 1500.0f;

	// true: 시작 시 방향 고정 / false: 플레이어 추적
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bFixDirectionAtStart = true;

	// 대쉬 방향 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	EDashDirection DashDirectionType = EDashDirection::Forward;

	// 추적 모드일 때 회전 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (EditCondition = "!bFixDirectionAtStart"))
	float TrackingRotationSpeed = 5.0f;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("Dash"); }
	
	void UpdateDashDirection(const FVector& LookDir);
private:
	FVector DashDirection;
	float OriginalMaxWalkSpeed = 0.0f;
};
