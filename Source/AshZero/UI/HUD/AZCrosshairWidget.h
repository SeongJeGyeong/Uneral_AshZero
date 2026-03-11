// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AZCrosshairWidget.generated.h"


UENUM(BlueprintType)
enum class ECrosshairType : uint8
{
    Default,    // 일반
    Aiming      // 견착
};

UCLASS()
class ASHZERO_API UAZCrosshairWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 크로스헤어 퍼짐 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair")
    void SetSpread(float Spread);

    // 견착 모드 전환
    UFUNCTION(BlueprintCallable, Category = "Crosshair")
    void SetCrosshairType(ECrosshairType NewType);

    // 반동 오프셋 적용
    UFUNCTION(BlueprintCallable, Category = "Crosshair")
    void SetRecoilOffset(FVector2D Offset);

protected:
    // 회전용 컨테이너
    UPROPERTY(meta = (BindWidget))
    class UCanvasPanel* CrosshairContainer;

    UPROPERTY(meta = (BindWidget))
    class UImage* Image_Top;

    UPROPERTY(meta = (BindWidget))
    class UImage* Image_Bottom;

    UPROPERTY(meta = (BindWidget))
    class UImage* Image_Left;

    UPROPERTY(meta = (BindWidget))
    class UImage* Image_Right;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    class UCanvasPanel* RootPanel;

    // 기본 퍼짐 거리
    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    float DefaultSpread = 20.f;

    // 견착 시 퍼짐 거리
    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    float AimingSpread = 20.f;

    // 현재 퍼짐
    float CurrentSpread = 20.f;

    // 현재 타입
    ECrosshairType CurrentType = ECrosshairType::Default;

    // 현재 회전 각도
    float CurrentRotation = 0.f;
    float TargetRotation = 0.f;

};