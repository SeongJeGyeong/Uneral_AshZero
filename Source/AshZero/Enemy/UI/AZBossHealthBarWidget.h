// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZBossHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UWidgetAnimation;

/**
 * 보스 HP 바 위젯 (화면 하단 고정)
 * 다크소울 스타일 딜레이 바 효과 지원
 */
UCLASS()
class ASHZERO_API UAZBossHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** 보스 정보 초기화 */
    UFUNCTION(BlueprintCallable, Category = "Boss")
    void InitBoss(const FText& InBossName, float MaxHealth);

    /** HP 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Boss")
    void UpdateHealth(float CurrentHealth, float MaxHealth);

    /** 보스 UI 표시 */
    UFUNCTION(BlueprintCallable, Category = "Boss")
    void ShowBossHealthBar();

    /** 보스 UI 숨김 */
    UFUNCTION(BlueprintCallable, Category = "Boss")
    void HideBossHealthBar();

    /** 보스 처치 연출 */
    UFUNCTION(BlueprintCallable, Category = "Boss")
    void PlayDefeatAnimation();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ===== UI 컴포넌트 (BP에서 바인딩) =====

    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    /** 딜레이 바 (데미지 후 천천히 감소) */
    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar* DelayedHealthBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* BossNameText;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* HealthPercentText;

    // ===== 애니메이션 (BP에서 설정) =====

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    UWidgetAnimation* ShowAnimation;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    UWidgetAnimation* HideAnimation;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    UWidgetAnimation* DefeatAnimation;

    // ===== 설정 =====

    /** 딜레이 바 시작 지연 시간 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DelayedBarStartDelay = 0.5f;

    /** 딜레이 바 감소 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DelayedBarDecreaseSpeed = 1.0f;

    /** HP 바 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLinearColor HealthBarColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);

    /** 딜레이 바 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLinearColor DelayedBarColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* AccumulatedDamageText;

private:
    float TargetHealthPercent = 1.0f;
    float DelayedHealthPercent = 1.0f;
    float DelayedBarTimer = 0.0f;
    bool bShouldUpdateDelayedBar = false;

    float AccumulatedDamage = 0.0f;
};