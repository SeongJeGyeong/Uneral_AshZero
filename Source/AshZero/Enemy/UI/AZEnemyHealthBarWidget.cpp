// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/UI/AZEnemyHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UAZEnemyHealthBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (HealthBar)
    {
        HealthBar->SetPercent(1.0f);
        HealthBar->SetFillColorAndOpacity(FullHealthColor);
    }

    // 초기에는 숨김
    SetVisibility(ESlateVisibility::Collapsed);
}

void UAZEnemyHealthBarWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
    if (MaxHealth <= 0.0f) return;

    float Percent = FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);

    if (HealthBar)
    {
        HealthBar->SetPercent(Percent);

        // HP 비율에 따라 색상 변경
        FLinearColor TargetColor = (Percent <= LowHealthThreshold) ?
            LowHealthColor : FullHealthColor;
        HealthBar->SetFillColorAndOpacity(TargetColor);
    }

    // 텍스트 업데이트 (선택적)
    if (HealthText)
    {
        HealthText->SetText(FText::FromString(
            FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth)
        ));
    }

    // 자동 숨김 타이머 시작
    ShowHealthBar();
    StartHideTimer();
}

void UAZEnemyHealthBarWidget::ShowHealthBar()
{
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UAZEnemyHealthBarWidget::HideHealthBar()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

void UAZEnemyHealthBarWidget::StartHideTimer()
{
    if (AutoHideDelay <= 0.0f) return;

    // 기존 타이머 취소
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);

        GetWorld()->GetTimerManager().SetTimer(
            HideTimerHandle,
            this,
            &UAZEnemyHealthBarWidget::HideHealthBar,
            AutoHideDelay,
            false
        );
    }
}