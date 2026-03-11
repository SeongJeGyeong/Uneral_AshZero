// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZEnemyHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 일반 몬스터 머리 위에 표시되는 HP 바
 * WidgetComponent로 월드 스페이스에 배치
 */
UCLASS()
class ASHZERO_API UAZEnemyHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** HP 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void UpdateHealth(float CurrentHealth, float MaxHealth);

    /** 위젯 표시 */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ShowHealthBar();

    /** 위젯 숨김 */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void HideHealthBar();

protected:
    virtual void NativeConstruct() override;

    // ===== UI 컴포넌트 (BP에서 바인딩) =====

    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* HealthText;

    // ===== 설정 =====

    /** HP 바 색상 (풀 HP) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Settings")
    FLinearColor FullHealthColor = FLinearColor(0.0f, 0.8f, 0.0f, 1.0f);

    /** HP 바 색상 (낮은 HP) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Settings")
    FLinearColor LowHealthColor = FLinearColor(0.8f, 0.0f, 0.0f, 1.0f);

    /** 낮은 HP 임계값 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Settings")
    float LowHealthThreshold = 0.3f;

    /** 자동 숨김 시간 (0이면 숨기지 않음) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AZ|Settings")
    float AutoHideDelay = 3.0f;

private:
    FTimerHandle HideTimerHandle;
    void StartHideTimer();
};