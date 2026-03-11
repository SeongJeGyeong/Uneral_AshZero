// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AZEnemyHealthUIComponent.generated.h"

class UWidgetComponent;
class UAZEnemyHealthBarWidget;
class UAZBossHealthBarWidget;

/**
 * 적 HP UI 관리 컴포넌트
 *
 * 일반 몬스터: WidgetComponent로 머리 위에 표시 (빌보드)
 * 보스: 플레이어 화면 하단에 고정
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ASHZERO_API UAZEnemyHealthUIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAZEnemyHealthUIComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
    // ===== 인터페이스 =====

    /** HP 변경 시 호출 (HealthComponent와 바인딩) */
    UFUNCTION(BlueprintCallable, Category = "AZ|UI")
    void OnHealthChanged(float CurrentHealth, float MaxHealth);

    /** 보스 UI 표시 (보스전 시작 트리거) */
    UFUNCTION(BlueprintCallable, Category = "AZ|UI")
    void ActivateBossUI();

	/** 보스 UI 표시 (모든 클라이언트) */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastShowBossUI();

	/** 보스 HP 업데이트 (모든 클라이언트) */
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastUpdateBossHealth(float CurrentHealth, float MaxHealth);

	/** 보스 처치 UI 연출 (모든 클라이언트) */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastBossDefeated();

	/** 보스 UI 숨김 (모든 클라이언트) */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastHideBossUI();

    /** 보스 처치 연출 */
    UFUNCTION(BlueprintCallable, Category = "AZ|UI")
    void OnBossDefeated();

    /** 사망 시 UI 정리 */
    UFUNCTION(BlueprintCallable, Category = "AZ|UI")
    void OnDeath();

	/**몬스터 HP 바 업데이트 */
    UFUNCTION()
    void UpdateHealthBar(float CurrentHealth, float MaxHealth);

    // ===== 설정 (BP에서 편집) =====

    /** 보스 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Settings")
    bool bIsBoss = false;

    /** 보스 이름 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Settings|Boss", meta = (EditCondition = "bIsBoss"))
    FText BossName = FText::FromString(TEXT("BOSS"));

	/** 보스 HP 위젯 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Settings|Boss", meta = (EditCondition = "bIsBoss"))
    TSubclassOf<UAZBossHealthBarWidget> BossWidgetClass;

    /** 일반 몬스터 HP 위젯 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Settings|Enemy", meta = (EditCondition = "!bIsBoss"))
    TSubclassOf<UAZEnemyHealthBarWidget> EnemyWidgetClass;

    /** HP 바 DrawSize */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Settings|Enemy", meta = (EditCondition = "!bIsBoss"))
    FVector2D HealthBarSize = FVector2D(120.0f, 15.0f);

    UPROPERTY(EditDefaultsOnly, Category = "AZ|UI")
    float FixedHeightOffset = 120.0f;  // 캡슐 위 고정 높이

    // 거리 기반 스케일
    UPROPERTY(EditDefaultsOnly, Category = "AZ|UI")
    bool bScaleByDistance = true;

    UPROPERTY(EditDefaultsOnly, Category = "AZ|UI")
    float MinDrawDistance = 500.0f;  // 이 거리 이하면 풀 사이즈

    UPROPERTY(EditDefaultsOnly, Category = "AZ|UI")
    float MaxDrawDistance = 3000.0f;  // 이 거리 이상이면 숨김

    UPROPERTY(EditDefaultsOnly, Category = "AZ|UI")
    float MinScale = 0.3f;  // 최소 스케일


private:
    void SetupEnemyUI();
    void SetupBossUI();

    UPROPERTY()
    UWidgetComponent* HealthWidgetComp;

    UPROPERTY()
    UAZEnemyHealthBarWidget* EnemyWidget;

    UPROPERTY()
    UAZBossHealthBarWidget* BossWidget;

    float CachedMaxHealth = 100.0f;
};