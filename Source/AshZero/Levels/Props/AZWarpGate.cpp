// Fill out your copyright notice in the Description page of Project Settings.


#include "Levels/Props/AZWarpGate.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponentCube.h"
#include "Character/AZPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "System/Player/AZPlayerController.h"
#include "System/AZSessionSubsystem.h"
#include "GameState/AZInGameGameState.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Weapon/AZWeapon.h"

// Sets default values
AAZWarpGate::AAZWarpGate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("RootScene");
	RootComponent = Root;

	Trigger = CreateDefaultSubobject<UBoxComponent>("Trigger");
	Trigger->SetupAttachment(RootComponent);

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>("Gate");
	GateMesh->SetupAttachment(RootComponent);

	RenderCaptureCube = CreateDefaultSubobject<USceneCaptureComponentCube>("RenderCaptureCube");
	RenderCaptureCube->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AAZWarpGate::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AAZWarpGate::OnTriggerBeginOverlap);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &AAZWarpGate::OnTriggerEndOverlap);
}

void AAZWarpGate::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AAZPlayerCharacter* PlayerCharacter = Cast<AAZPlayerCharacter>(OtherActor))
	{
		OverlappedCharacters.AddUnique(PlayerCharacter);
		if (!HasAuthority()) return;

		EvaluateWarpProgress_Server();
	}
}

void AAZWarpGate::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AAZPlayerCharacter* PlayerCharacter = Cast<AAZPlayerCharacter>(OtherActor))
	{
		OverlappedCharacters.Remove(PlayerCharacter);
		if (!HasAuthority()) return;

		EvaluateWarpProgress_Server();
	}
}

void AAZWarpGate::StartWarpProgress()
{
	if (bWarpInProgress)
		return;

	bWarpInProgress = true;
	WarpElapsedTime = 0.f;

	CreateWarpWidget_Multicast();

	// 모든 클라이언트에게 UI 표시
	ShowWarpUI_Multicast(true);

	GetWorldTimerManager().SetTimer(
		WarpTimerHandle,
		this,
		&AAZWarpGate::UpdateWarpProgress,
		0.01f,
		true
	);
}

void AAZWarpGate::UpdateWarpProgress()
{
	WarpElapsedTime += 0.01f;

	const float Progress = FMath::Clamp(WarpElapsedTime / WarpDuration, 0.f, 1.f);

	// 모든 클라이언트에 진행도 전달
	UpdateWarpProgress_Multicast(Progress);

	if (WarpElapsedTime >= WarpDuration)
	{
		CompleteWarp();
	}

}

void AAZWarpGate::ResetWarpProgress()
{
	if (!bWarpInProgress)
		return;

	bWarpInProgress = false;
	WarpElapsedTime = 0.f;

	GetWorldTimerManager().ClearTimer(WarpTimerHandle);

	// 모든 클라이언트 UI 숨김 + 초기화
	ShowWarpUI_Multicast(false);
	UpdateWarpProgress_Multicast(0.f);
}

void AAZWarpGate::CompleteWarp()
{
	GetWorldTimerManager().ClearTimer(WarpTimerHandle);
	bWarpInProgress = false;

	ShowWarpUI_Multicast(false);

	CompleteWarp_Multicast();
}

void AAZWarpGate::CompleteWarp_Multicast_Implementation()
{
	bool bServerFlag = false;
	for (AAZPlayerCharacter* Character : OverlappedCharacters)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TeleportNiagara, Character->GetActorLocation(), Character->GetActorRotation(), FVector(1.f));
		Character->SetActorEnableCollision(false);
		Character->SetActorHiddenInGame(true);
		if (Character->CurrentWeapon)
			Character->CurrentWeapon->SetActorHiddenInGame(true);
		if (HasAuthority() && !bServerFlag && NiagaraComp)
		{
			NiagaraComp->OnSystemFinished.AddDynamic(this, &AAZWarpGate::WarpFinished);
			bServerFlag = true;
		}
	}
}

void AAZWarpGate::EvaluateWarpProgress_Server_Implementation()
{
	AAZInGameGameState* GameState = GetWorld()->GetGameState<AAZInGameGameState>();
	if (!GameState) return;

	const int32 AlivePlayers = GameState->GetAlivePlayers();
	const int32 Overlapped = OverlappedCharacters.Num();

	if (AlivePlayers <= 0)
	{
		ResetWarpProgress();
		return;
	}

	if (Overlapped >= AlivePlayers)
	{
		if (!bWarpInProgress)
			StartWarpProgress();

		return;
	}

	ResetWarpProgress();
}

void AAZWarpGate::WarpFinished(UNiagaraComponent* FinishedComponent)
{
	if (!HasAuthority()) return;

	if (AAZPlayerController* PlayerController = Cast<AAZPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (UAZSessionSubsystem* SessionSystem = GetGameInstance()->GetSubsystem<UAZSessionSubsystem>())
		{
			SessionSystem->CurrentGameState = EGameState::Lobby;
			PlayerController->PreTravelCleanUp_Client();
			SessionSystem->ExitSession();
		}
	}
}

void AAZWarpGate::CreateWarpWidget_Multicast_Implementation()
{
	if (!GetWorld())
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC))
	{
		AZPC->CreateWarpWidget();
	}
}

void AAZWarpGate::ShowWarpUI_Multicast_Implementation(bool bShow)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC))
		{
			AZPC->ShowWarpUI(bShow);
		}
	}
}

void AAZWarpGate::UpdateWarpProgress_Multicast_Implementation(float Progress)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC))
		{
			AZPC->UpdateWarpProgress(Progress);
		}
	}
}