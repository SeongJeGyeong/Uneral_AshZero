// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AZExitUI.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"

void UAZExitUI::NativeConstruct()
{
	Super::NativeConstruct();
	SetPercent(0.f);
}

void UAZExitUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UAZExitUI::SetPercent(float Figure)
{
	if (!ProgressBarMID)
	{
		ProgressBarMID = ProgressGuage->GetDynamicMaterial();
	}

	Percent = FMath::Clamp(Figure, 0.f, 1.f);
	ProgressBarMID->SetScalarParameterValue(TEXT("Percent"), Percent);

	const int32 NewIndex = FMath::Clamp(FMath::FloorToInt(Percent * MaxStepCount), 0, MaxStepCount - 1);

	// 경계 통과 시에만 갱신
	if (NewIndex != CachedIndex)
	{
		WidgetSwitcher->SetActiveWidgetIndex(NewIndex);
		CachedIndex = NewIndex;
	}
}