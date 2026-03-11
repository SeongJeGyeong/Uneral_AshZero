// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/UI/AZBossHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "TimerManager.h"

void UAZBossHealthBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TargetHealthPercent = 1.0f;
    DelayedHealthPercent = 1.0f;
    AccumulatedDamage = 0.0f;

    if (HealthBar)
    {
        HealthBar->SetPercent(1.0f);
        HealthBar->SetFillColorAndOpacity(HealthBarColor);
    }

    if (DelayedHealthBar)
    {
        DelayedHealthBar->SetPercent(1.0f);
        DelayedHealthBar->SetFillColorAndOpacity(DelayedBarColor);
    }

    if (AccumulatedDamageText)
    {
        AccumulatedDamageText->SetVisibility(ESlateVisibility::Collapsed);
    }

    SetVisibility(ESlateVisibility::Collapsed);
}

void UAZBossHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);


    if (bShouldUpdateDelayedBar && DelayedHealthBar)
    {
        DelayedBarTimer -= InDeltaTime;

        if (DelayedBarTimer <= 0.0f)
        {
            // 1. 보간 계산
            DelayedHealthPercent = FMath::FInterpConstantTo(
                DelayedHealthPercent,
                TargetHealthPercent,
                InDeltaTime,
                0.5f // 초당 50%씩 감소
            );

            if (FMath::IsNearlyEqual(DelayedHealthPercent, TargetHealthPercent, 0.0001f))
            {
                DelayedHealthPercent = TargetHealthPercent;
                bShouldUpdateDelayedBar = false;

                AccumulatedDamage = 0.0f;
                if (AccumulatedDamageText)
                {
                    AccumulatedDamageText->SetVisibility(ESlateVisibility::Collapsed);
                }

            }

            // 3. UI 반영
            DelayedHealthBar->SetPercent(DelayedHealthPercent);
        }
    }
}

void UAZBossHealthBarWidget::InitBoss(const FText& InBossName, float MaxHealth)
{
    if (BossNameText) BossNameText->SetText(InBossName);

    TargetHealthPercent = 1.0f;
    DelayedHealthPercent = 1.0f;
    AccumulatedDamage = 0.0f;

    if (HealthBar)HealthBar->SetPercent(1.0f);
    
    if (DelayedHealthBar)DelayedHealthBar->SetPercent(1.0f);

    if (AccumulatedDamageText)AccumulatedDamageText->SetVisibility(ESlateVisibility::Collapsed);
}

void UAZBossHealthBarWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
    if (MaxHealth <= 0.0f) return;

    float NewPercent = FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);

    // HP 바 즉시 업데이트
    if (HealthBar)HealthBar->SetPercent(NewPercent);
   
    // 데미지를 받았을 때만 딜레이 바 효과 시작
    if (NewPercent < TargetHealthPercent)
    {
        float DamageTaken = (TargetHealthPercent - NewPercent) * MaxHealth;
        AccumulatedDamage += DamageTaken;

        // ★ 누적 데미지 텍스트 업데이트
        if (AccumulatedDamageText)
        {
            AccumulatedDamageText->SetText(FText::FromString(
                FString::Printf(TEXT("-%.0f"), AccumulatedDamage)
            ));
            AccumulatedDamageText->SetVisibility(ESlateVisibility::HitTestInvisible);
        }

        DelayedBarTimer = DelayedBarStartDelay;
        bShouldUpdateDelayedBar = true;
    }

    TargetHealthPercent = NewPercent;

    // 퍼센트 텍스트 업데이트
    if (HealthPercentText)
    {
        int32 PercentInt = FMath::RoundToInt(NewPercent * 100.0f);
        HealthPercentText->SetText(FText::FromString(
            FString::Printf(TEXT("%d%%"), PercentInt)
        ));
    }
}

void UAZBossHealthBarWidget::ShowBossHealthBar()
{
    SetVisibility(ESlateVisibility::HitTestInvisible);

    if (ShowAnimation)
    {
        PlayAnimation(ShowAnimation);
    }
}

void UAZBossHealthBarWidget::HideBossHealthBar()
{
    if (HideAnimation)
    {
        PlayAnimation(HideAnimation);

        // 애니메이션 끝나면 숨기기
        FTimerHandle TimerHandle;
        float Duration = HideAnimation->GetEndTime();

        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            [this]() { SetVisibility(ESlateVisibility::Collapsed); },
            Duration,
            false
        );
    }
    else
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UAZBossHealthBarWidget::PlayDefeatAnimation()
{
    if (DefeatAnimation)
    {
        PlayAnimation(DefeatAnimation);
    }

    // 애니메이션 후 숨기기
    float Delay = DefeatAnimation ? DefeatAnimation->GetEndTime() + 1.5f : 2.0f;

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        this,
        &UAZBossHealthBarWidget::HideBossHealthBar,
        Delay,
        false
    );
}