// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AZEnemyFSM.generated.h"

class AAZPlayerCharacter;

// 사용할 상태 정의
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Idle	UMETA(DisplayName = "Idle"),	
    Patrol	UMETA(DisplayName = "Patrol"),	
    Chase	UMETA(DisplayName = "Chase"),	
    Attack	UMETA(DisplayName = "Attack"),	
    Damage	UMETA(DisplayName = "Damage"),	
    Die		UMETA(DisplayName = "Die")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHZERO_API UAZEnemyFSM : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAZEnemyFSM();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/* == 상태 머신 == */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AZ|FSM")
	EEnemyState State = EEnemyState::Idle;

	UFUNCTION(Server, Reliable)
	void ServerSetState(EEnemyState NewState);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastStateChanged(EEnemyState NewState);
    
	/* == 감지,추적 설정 == */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Detection")
    float DetectionRange = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Detection")
    float LoseTargetRange = 800.0f;

    /* == 패트롤 설정 == */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Patrol")
    float PatrolRadius = 500.0f;

    // 순찰 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Patrol")
    float PatrolSpeed = 150.0f;

    // 순찰 지점 도착 후 대기 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Patrol")
    float PatrolWaitTime = 2.0f;

    /* == 전투 설정 == */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Combat")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Combat")
    float AttackAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Movement")
    float ChaseSpeed = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Combat")
    float Damage = 10.0f;

    UPROPERTY(BlueprintReadOnly, Category = "AZ|Combat")
    bool bIsAttacking = false;

    // ===== 피격 설정 =====
    // 피격 경직 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Damage")
    float DamageStunTime = 1.5f;

    // 피격 넘어짐 체력 임계값 (이 % 이하로 떨어지면 1회 넘어짐)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Damage")
    float DamageThresholdPercent = 0.5f;

    // 이미 넘어짐 애니메이션 재생했는지
    UPROPERTY(BlueprintReadOnly, Category = "AZ|Damage")
    bool bHasPlayedKnockdown = false;


    // ===== 외부 인터페이스 =====
public:
    // 데미지 받았을 때 호출
    UFUNCTION(BlueprintCallable, Category = "AZ|FSM")
    void OnDamageReceived(float DamageAmount, AActor* DamageCauser);

    // 타겟 강제 설정 (피격 시 사용)
    UFUNCTION(BlueprintCallable, Category = "AZ|FSM")
    void SetTarget(AAZPlayerCharacter* NewTarget);

    // 공격 가능 여부
    UFUNCTION(BlueprintPure, Category = "AZ|FSM")
    bool CanAttackTarget() const;

    // 현재 타겟
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "AZ|FSM")
    AAZPlayerCharacter* CurrentTarget;

    UFUNCTION(BlueprintCallable, Category = "AZ|FSM")
    AActor* GetTargetActor() const;

protected:
    // ===== 내부 변수 =====
    UPROPERTY()
    class AAZEnemyBase* Owner;
     
    UPROPERTY()
    class AAIController* AIController;

    // 순찰 관련
    FVector PatrolCenter;		// 순찰 중심점 (BeginPlay 시 설정)
    FVector PatrolLocation;		// 현재 목표 순찰 지점
    float StateTimer = 0.0f;
    float AttackTimer = 0.0f;

    // ===== 상태 함수 =====
    void UpdateStateMachine(float DeltaTime);
    void IdleState(float DeltaTime);
    void PatrolState(float DeltaTime);
    void ChaseState(float DeltaTime);
    void AttackState(float DeltaTime);
    void DamageState(float DeltaTime);
    void DieState(float DeltaTime);

    // ===== 유틸리티 =====
    // 범위 내 플레이어 찾기
    AAZPlayerCharacter* FindPlayerInRange(float Range) const;

    // 랜덤 순찰 위치 구하기
    bool GetRandomPatrolLocation(FVector& OutLocation);

    // 타겟과의 거리
    float GetDistanceToTarget() const;

    // 공격 몽타주 종료 콜백
    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
