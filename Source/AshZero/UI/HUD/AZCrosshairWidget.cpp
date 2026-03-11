// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AZCrosshairWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UAZCrosshairWidget::SetSpread(float Spread)
{
    CurrentSpread = Spread;

    if (Image_Top)
    {
        if (UCanvasPanelSlot* TopSlot = Cast<UCanvasPanelSlot>(Image_Top->Slot))
        {
            TopSlot->SetPosition(FVector2D(0, -CurrentSpread));
        }
    }
    if (Image_Bottom)
    {
        if (UCanvasPanelSlot* BottomSlot = Cast<UCanvasPanelSlot>(Image_Bottom->Slot))
        {
            BottomSlot->SetPosition(FVector2D(0, CurrentSpread));
        }
    }
    if (Image_Left)
    {
        if (UCanvasPanelSlot* LeftSlot = Cast<UCanvasPanelSlot>(Image_Left->Slot))
        {
            LeftSlot->SetPosition(FVector2D(-CurrentSpread, 0));
        }
    }
    if (Image_Right)
    {
        if (UCanvasPanelSlot* RightSlot = Cast<UCanvasPanelSlot>(Image_Right->Slot))
        {
            RightSlot->SetPosition(FVector2D(CurrentSpread, 0));
        }
    }
}
void UAZCrosshairWidget::SetCrosshairType(ECrosshairType NewType)
{
    CurrentType = NewType;

	// 퍼짐 조절
    float TargetSpread = (NewType == ECrosshairType::Aiming) ? AimingSpread : DefaultSpread;
    SetSpread(TargetSpread);

	// 45도 회전
    float Rotation = (NewType == ECrosshairType::Aiming) ? 45.f : 0.f;
    if (CrosshairContainer)
    {
        CrosshairContainer->SetRenderTransformAngle(Rotation);
    }


    // 색상 변경
    /*
    FLinearColor Color = (NewType == ECrosshairType::Aiming) ? FLinearColor::Red : FLinearColor::White;
    if (Image_Top) Image_Top->SetColorAndOpacity(Color);
    if (Image_Bottom) Image_Bottom->SetColorAndOpacity(Color);
    if (Image_Left) Image_Left->SetColorAndOpacity(Color);
    if (Image_Right) Image_Right->SetColorAndOpacity(Color);
    */
}
void UAZCrosshairWidget::SetRecoilOffset(FVector2D Offset)
{
    if (RootPanel)
    {
        RootPanel->SetRenderTranslation(Offset);
    }
}