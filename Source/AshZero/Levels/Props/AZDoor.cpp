// Fill out your copyright notice in the Description page of Project Settings.


#include "Levels/Props/AZDoor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Levels/Rooms/AZBaseRoom.h"
#include "Character/AZPlayerCharacter.h"
#include "System/Subsystems/AZSoundManagerSubsystem.h"

AAZDoor::AAZDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("Root Scene");
	RootComponent = Root;

	Trigger = CreateDefaultSubobject<UBoxComponent>("Trigger");
	Trigger->SetupAttachment(RootComponent);

	Door = CreateDefaultSubobject<UStaticMeshComponent>("Door");
	Door->SetupAttachment(RootComponent);

	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>("DoorFrame");
	DoorFrame->SetupAttachment(RootComponent);
}

void AAZDoor::BeginPlay()
{
	Super::BeginPlay();
	
	Trigger->OnComponentBeginOverlap.RemoveDynamic(this, &AAZDoor::OnTriggerBeginOverlap);
	Trigger->OnComponentEndOverlap.RemoveDynamic(this, &AAZDoor::OnTriggerEndOverlap);

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AAZDoor::OnTriggerBeginOverlap);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &AAZDoor::OnTriggerEndOverlap);
	if (Door)
	{
		if (!DoorMID) DoorMID = Door->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (DoorFrame)
	{
		if (!DoorFrameMID) DoorFrameMID = DoorFrame->CreateAndSetMaterialInstanceDynamic(0);
	}
}

void AAZDoor::SetDoorTrigger(bool bActivate)
{
	Trigger->SetCollisionEnabled(bActivate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	if (!bActivate)
	{
		if (!DoorMID) DoorMID = Door->CreateAndSetMaterialInstanceDynamic(0);
		if (!DoorFrameMID) DoorFrameMID = DoorFrame->CreateAndSetMaterialInstanceDynamic(0);
		if (DoorMID)
		{
			DoorMID->SetVectorParameterValue(TEXT("EmissiveFactor"), ClosedColor);
			DoorMID->SetScalarParameterValue(TEXT("EmissiveStrength"), NormalEmissive);
		}
		if (DoorFrameMID)
		{
			DoorFrameMID->SetVectorParameterValue(TEXT("EmissiveFactor"), ClosedColor);
			DoorFrameMID->SetScalarParameterValue(TEXT("EmissiveStrength"), NormalEmissive);
		}
	}
}

void AAZDoor::SensingDoor(bool bSense)
{
	if (bSense)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(CloseDelayTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(CloseDelayTimerHandle);
		}

		if (DoorMID)
		{
			DoorMID->SetVectorParameterValue(TEXT("EmissiveFactor"), OpenColor);
			DoorMID->SetScalarParameterValue(TEXT("EmissiveStrength"), RadiateEmissive);
		}
		if (DoorFrameMID)
		{
			DoorFrameMID->SetVectorParameterValue(TEXT("EmissiveFactor"), OpenColor);
			DoorFrameMID->SetScalarParameterValue(TEXT("EmissiveStrength"), RadiateEmissive);
		}

		GetWorld()->GetTimerManager().ClearTimer(OpenDelayTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(OpenDelayTimerHandle, this, &AAZDoor::ExecuteOpen, OpenDelayTime, false);
	}
	else
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(OpenDelayTimerHandle))
		{
			// 타이머 취소 (문 열지 않음)
			GetWorld()->GetTimerManager().ClearTimer(OpenDelayTimerHandle);
		}
		
		if(bIsDoorOpen)
		{
			GetWorld()->GetTimerManager().SetTimer(CloseDelayTimerHandle, this, &AAZDoor::ExecuteClose, OpenDelayTime, false);
		}
		// 색상 원상복구 (필요시)
		if (DoorMID)
		{
			DoorMID->SetVectorParameterValue(TEXT("EmissiveFactor"), NormalColor);
			DoorMID->SetScalarParameterValue(TEXT("EmissiveStrength"), NormalEmissive);
		}
		if (DoorFrameMID)
		{
			DoorFrameMID->SetVectorParameterValue(TEXT("EmissiveFactor"), NormalColor);
			DoorFrameMID->SetScalarParameterValue(TEXT("EmissiveStrength"), NormalEmissive);
		}
	}
}

void AAZDoor::ExecuteOpen()
{
	UAZSoundManagerSubsystem* SoundManager = GetGameInstance()->GetSubsystem<UAZSoundManagerSubsystem>();
	if (SoundManager) SoundManager->PlaySFX(this, ESFXType::Door_Open);
	GetWorld()->GetTimerManager().ClearTimer(OpenDelayTimerHandle);
	bIsDoorOpen = true;
	OnDoorSensedEffect(true);
}

void AAZDoor::ExecuteClose()
{
	UAZSoundManagerSubsystem* SoundManager = GetGameInstance()->GetSubsystem<UAZSoundManagerSubsystem>();
	if (SoundManager) SoundManager->PlaySFX(this, ESFXType::Door_Open);
	GetWorld()->GetTimerManager().ClearTimer(CloseDelayTimerHandle);
	bIsDoorOpen = false;

	// 색상 끄기
	if (DoorMID)
	{
		DoorMID->SetVectorParameterValue(TEXT("EmissiveFactor"), NormalColor);
		DoorMID->SetScalarParameterValue(TEXT("EmissiveStrength"), NormalEmissive);
	}
	if (DoorFrameMID)
	{
		DoorFrameMID->SetVectorParameterValue(TEXT("EmissiveFactor"), NormalColor);
		DoorFrameMID->SetScalarParameterValue(TEXT("EmissiveStrength"), NormalEmissive);
	}

	// 블루프린트에 구현된 이벤트 호출 (Timeline 역재생용)
	OnDoorSensedEffect(false);
}

void AAZDoor::CheckDoorState()
{
	if (bIsMoving) return;

	if (OverlappedCount > 0)
	{
		if (!bIsDoorOpen)
		{
			SensingDoor(true);
		}
	}
	else
	{
		if (bIsDoorOpen)
		{
			SensingDoor(false);
		}
	}
}

void AAZDoor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	++OverlappedCount;
	CheckDoorState();
}

void AAZDoor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	--OverlappedCount;
	CheckDoorState();
}

void AAZDoor::OnDoorActive(bool bDoorOpen)
{
	if (!bDoorOpen)
	{
		FVector NewLocation = Door->GetRelativeLocation();
		NewLocation.Z = 0.f;
		Door->SetRelativeLocation(NewLocation);
		GetWorld()->GetTimerManager().ClearTimer(OpenDelayTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(CloseDelayTimerHandle);
	}

	SetDoorTrigger(bDoorOpen);
}