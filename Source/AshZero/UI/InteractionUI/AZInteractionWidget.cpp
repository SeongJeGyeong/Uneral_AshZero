// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InteractionUI/AZInteractionWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetInputLibrary.h"

void UAZInteractionWidget::UpdatePositionInViewport(FMargin ScreenMargin, float ScreenRadiusPercent)
{
	if (!GEngine || !GEngine->GameViewport) return;

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !PlayerController->PlayerCameraManager) return;

	// 타겟 위치 갱신
	if (TargetActor)
	{
		LastWorldLocation = TargetActor->GetActorLocation();
	}

	// 뷰포트 정보
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	FVector2D ViewportCenter = ViewportSize * 0.5f;

	// 카메라 정보
	const FVector CameraLocation =
		PlayerController->PlayerCameraManager->GetCameraLocation();
	const FVector CameraForward =
		PlayerController->PlayerCameraManager->GetActorForwardVector();

	const FVector ToTarget =
		(LastWorldLocation - CameraLocation).GetSafeNormal();

	const float Dot =
		FVector::DotProduct(CameraForward, ToTarget);

	const bool bIsInFront = Dot > 0.f;

	// 월드 → 스크린
	FVector2D ScreenLocation;
	PlayerController->ProjectWorldLocationToScreen(
		LastWorldLocation,
		ScreenLocation,
		true
	);

	// 카메라 뒤에 있으면 → 화면 가장자리
	if (!bIsInFront)
	{
		FVector2D Dir = (ScreenLocation - ViewportCenter).GetSafeNormal();
		if (Dir.IsNearlyZero())
		{
			Dir = FVector2D(0.f, -1.f);
		}

		float Radius =
			ViewportCenter.Y * FMath::Clamp(ScreenRadiusPercent, 0.f, 1.f);

		ScreenLocation = ViewportCenter + Dir * Radius;
	}

	// 마진 기준 클램프
	ScreenLocation.X = FMath::Clamp(
		ScreenLocation.X,
		ScreenMargin.Left,
		ViewportSize.X - ScreenMargin.Right
	);

	ScreenLocation.Y = FMath::Clamp(
		ScreenLocation.Y,
		ScreenMargin.Top,
		ViewportSize.Y - ScreenMargin.Bottom
	);

	// ★ 핵심 수정 포인트 ★
	// 절대 좌표로 배치
	SetPositionInViewport(ScreenLocation, true);

	//if (!GEngine || !GEngine->GameViewport) return;

	//FVector2D ViewportHalfSize;
	//GEngine->GameViewport->GetViewportSize(ViewportHalfSize);
	//ViewportHalfSize *= 0.5f;

	//APlayerController* PlayerController = GetOwningPlayer();
	//if (!PlayerController) return;

	//FVector WorldPosition;
	//FVector WorldDirection;
	//PlayerController->DeprojectScreenPositionToWorld(ViewportHalfSize.X, ViewportHalfSize.Y, WorldPosition, WorldDirection);

	//if (TargetActor)
	//{
	//	LastWorldLocation = TargetActor->GetActorLocation();
	//}

	//FRotator LookAtRotation = FRotationMatrix::MakeFromX(LastWorldLocation - WorldPosition).Rotator();
	//LookAtRotation.Vector().ForwardVector;
	//LookAtRotation.Vector().UpVector;
	//LookAtRotation.Vector().RightVector;

	//FVector CameraForwardVector = PlayerController->PlayerCameraManager->GetActorForwardVector();
	//FVector CameraRightVector = PlayerController->PlayerCameraManager->GetActorRightVector();

	//double ForwardDot = CameraForwardVector.Dot(LookAtRotation.Vector().ForwardVector);
	//double RightDot = CameraRightVector.Dot(LookAtRotation.Vector().RightVector);

	//double ForwardMapRange = FMath::GetMappedRangeValueClamped(FVector2D(0.f, -1.f), FVector2D(0, -1.f), ForwardDot);
	//double RightMapRange = FMath::GetMappedRangeValueClamped(FVector2D(0.f, -1.f), FVector2D(0, 1.f), RightDot);

	//FVector WorldLocation = LookAtRotation.Vector().ForwardVector + WorldPosition;
	//WorldLocation += LookAtRotation.Vector().UpVector * ForwardMapRange;
	////WorldLocation += WorldRotation.Vector().RightVector

	//FVector2D ScreenLocation;
	//PlayerController->ProjectWorldLocationToScreen(WorldLocation, ScreenLocation, true);

	//FVector2D ScreenNormal = FVector2D(ScreenLocation.X - ViewportHalfSize.X, ScreenLocation.Y - ViewportHalfSize.Y).GetSafeNormal();
	//double ScreenDegreeX = UKismetMathLibrary::DegAcos(ScreenNormal.X);

	//UKismetMathLibrary::DegSin(ScreenDegreeX);

	//double ScreenRadiusMapRange = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 1.f), FVector2D(0, ViewportHalfSize.Y), ScreenRadiusPercent);

	//double X = UKismetMathLibrary::DegCos(ScreenDegreeX) * ScreenRadiusMapRange + ViewportHalfSize.X;
	//double Y = UKismetMathLibrary::DegSin(ScreenDegreeX) * ScreenRadiusMapRange * ((ScreenNormal.Y <= 0.f) ? -1.f : 1.f) + ViewportHalfSize.Y;

	//FVector2D DesiredSize = GetDesiredSize();

	//float ClampedX = FMath::Clamp(X, DesiredSize.X + FMath::Abs(ScreenMargin.Left), ViewportHalfSize.X - DesiredSize.X - FMath::Abs(ScreenMargin.Top));
	//float ClampedY = FMath::Clamp(Y, DesiredSize.Y + FMath::Abs(ScreenMargin.Right), ViewportHalfSize.Y - DesiredSize.Y - FMath::Abs(ScreenMargin.Bottom));
	//FVector2D ClampedVec = FVector2D(ClampedX, ClampedY);

	//FVector2D ResultScreenPosition;

	//if (IsOnScreen(ScreenMargin))
	//{
	//	PlayerController->ProjectWorldLocationToScreen(LastWorldLocation, ResultScreenPosition, true);
	//}
	//else
	//{
	//	ResultScreenPosition = ClampedVec;
	//}
	//
	//ResultScreenPosition -= ViewportHalfSize;
	//FGeometry WidgetGeometry = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this);

	//FVector2D LocalCoordinate;
	//USlateBlueprintLibrary::ScreenToWidgetLocal(this, WidgetGeometry, ResultScreenPosition, LocalCoordinate);

	//SetRenderTranslation(LocalCoordinate);
}

void UAZInteractionWidget::SetInteractionText(FKey Key, FText Info)
{
	if (Key.IsBindableInBlueprints() && (Key.IsGamepadKey() == false && Key.IsMouseButton() == false))
	{
		InteractionKey->SetText(Key.GetDisplayName().ToUpper());
	}
	else
	{
		InteractionKey->SetText(FText::FromString(TEXT("F")));
	}

	InfoText->SetText(Info);
}

void UAZInteractionWidget::SetInteractionPercent(float Percent)
{
}

bool UAZInteractionWidget::IsOnScreen(FMargin Margin)
{
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	FVector2D ViewportHalfSize = ViewportSize * 0.5f;

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController) return false;
	FVector2D ScreenLocation;
	PlayerController->ProjectWorldLocationToScreen(LastWorldLocation, ScreenLocation, true);

	float MinX = (ScreenLocation.X < ViewportHalfSize.X) ? Margin.Left : Margin.Right;
	float MinY = (ScreenLocation.Y < ViewportHalfSize.Y) ? Margin.Top : Margin.Bottom;

	return (UKismetMathLibrary::InRange_FloatFloat(ScreenLocation.X, MinX, ViewportSize.X - MinX) &&
			UKismetMathLibrary::InRange_FloatFloat(ScreenLocation.Y, MinY, ViewportSize.Y - MinY));
}
