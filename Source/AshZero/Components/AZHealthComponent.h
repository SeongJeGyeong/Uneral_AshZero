#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AZStatusTypes.h"
#include "AZHealthComponent.generated.h"

// 체력 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, CurrentHealth, float, MaxHealth);

// 사망 델리게이트 - 데스박스 생성 연결용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathDelegate, FVector, DeathLocation, AActor*, Killer);

// 생존 수치 변경 델리게이트 (허기, 갈증 통합)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSurvivalStatsChangedDelegate, float, CurrentFullness, float, MaxFullness, float, CurrentHydration, float, MaxHydration);

// 체력 변경 델리게이트에 데미지 가해자(Causer) 정보를 추가하거나 별도 델리게이트 생성
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHitRegisteredDelegate, float, Damage, AActor*, DamageCauser, FVector, HitLocation);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ASHZERO_API UAZHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAZHealthComponent();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 생존 수치 감소를 처리할 함수
    void HandleSurvivalUpdate();

public:

    // 기본 최대 체력 (원본 값, 버프/디버프 계산의 기준)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Health")
    float BaseMaxHp = 100.f;

	// ===== 체력 (Health) =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Hp, Category = "AZ|Health")
    float MaxHp;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Hp, Category = "AZ|Health")
    float Hp;

    UFUNCTION(BlueprintPure, Category = "AZ|Health")
    float GetHp() const { return Hp; }

    UFUNCTION(BlueprintPure, Category = "AZ|Health")
    float GetMaxHp() const { return MaxHp; }

    UPROPERTY(BlueprintReadOnly, Category = "AZ|Health")
    bool bIsDead = false;

    // 현재 적용 중인 MaxHp 배율 (1.0 = 100%) 건들지 마세요 직접 설정하는 수치 아닙니다.
    UPROPERTY(BlueprintReadOnly, Category = "AZ|Health")
    float CurrentMaxHpMultiplier = 1.0f;

    // ===== 무적 (Invincible) =====
    UPROPERTY(BlueprintReadOnly, Category = "AZ|Health")
    bool bIsInvincible = false;

    UFUNCTION(BlueprintCallable, Category = "AZ|Health")
    void SetInvincible(bool bNewInvincible);

    UFUNCTION(BlueprintPure, Category = "AZ|Health")
    bool IsInvincible() const { return bIsInvincible; }

    // ===== 허기 (Fullness) =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Survival")
    float MaxFullness = 100.f;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SurvivalStats, Category = "AZ|Survival")
    float CurrentFullness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Survival")
    float FullnessDecreaseAmount = 1.0f; // 한 번에 줄어들 양

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Survival")
    float FullnessTickInterval = 50.0f;   // 줄어드는 주기 (초)

    // ===== 갈증 (Hydration) =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Survival")
    float MaxHydration = 100.f;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SurvivalStats, Category = "AZ|Survival")
    float CurrentHydration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Survival")
    float HydrationDecreaseAmount = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Survival")
    float HydrationTickInterval = 50.0f;


	// ===== 생존 수치 임계값 및 효과 ===== (배열)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Survival")
    TArray<FSurvivalThreshold> FullnessThresholds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Survival")
    TArray<FSurvivalThreshold> HydrationThresholds;


    // 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "AZ|Health")
    FOnHealthChangedDelegate OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "AZ|Health")
    FOnDeathDelegate OnDeath;  // ← 데스박스 연결할 곳

    UPROPERTY(BlueprintAssignable, Category = "AZ|Events")
    FOnSurvivalStatsChangedDelegate OnSurvivalStatsChanged;

    UPROPERTY(BlueprintAssignable, Category = "AZ|Events")
    FOnHitRegisteredDelegate OnHitRegistered;

    // 유틸리티
    UFUNCTION(BlueprintPure, Category = "AZ|Health")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintPure, Category = "AZ|Health")
    float GetHealthPercent() const { return MaxHp > 0.f ? Hp / MaxHp : 0.f; }

    // 체력 설정 (서버 권한)
    UFUNCTION(BlueprintCallable, Category = "AZ|Health")
    void SetHealth(float NewHealth);

    UFUNCTION()
    void AddHealth(float HealthValue);

    UFUNCTION(Server, Reliable)
    void AddHealth_Sever(float HealthValue);

    // 갈증 추가
    UFUNCTION()
    void AddHydration(float HydrationthValue);

    UFUNCTION(Server, Reliable)
    void AddHydration_Sever(float HydrationthValue);

    UFUNCTION()
    void AddFullness(float FullnessValue);

    UFUNCTION(Server, Reliable)
    void AddFullness_Sever(float FullnessValue);

    // ===== MaxHp 배율 시스템 =====
    // 배율 적용 (Starvation 디버프, MaxHpUp 버프 등)
    UFUNCTION(BlueprintCallable, Category = "AZ|Health")
    void SetMaxHpMultiplier(float NewMultiplier);

    // 배율 초기화 (1.0으로 복구)
    UFUNCTION(BlueprintCallable, Category = "AZ|Health")
    void ResetMaxHpMultiplier();

    // 인터페이스를 통해 캐릭터가 호출해줄 함수
    void ApplyStatusEffect(const FAZStatusEffectPacket& Packet, AActor* Causer);

    // 특정 상태 이상 강제 종료
    void StopStatusEffect(EAZStatusType StatusType);
    
    UFUNCTION()
    void OnRep_Hp();

protected:
    UFUNCTION() 
    void OnRep_SurvivalStats();

    UFUNCTION()
    void OnTakeAnyDamageCallback(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastDeath(AActor* Killer);

    // 실제 틱 데미지 처리
    void ProcessDoTTick(EAZStatusType StatusType, AActor* Causer);

    // 해당 타입이 체력/생존 관련 DoT인지 판별
    bool IsHealthDoT(EAZStatusType Type) const;

private:
    FTimerHandle SurvivalTimerHandle;
    float FullnessElapsed = 0.f;
    float HydrationElapsed = 0.f;

    // 현재 활성화된 DoT 효과 관리
    UPROPERTY()
    TMap<EAZStatusType, FActiveDoTEffect> ActiveDoTEffects;

};