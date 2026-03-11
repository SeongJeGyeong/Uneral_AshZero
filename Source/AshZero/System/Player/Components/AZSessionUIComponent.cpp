// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Player/Components/AZSessionUIComponent.h"
#include "UI/Level/Lobby/AZLobbyUI.h"
#include "System/Player/AZPlayerController.h"
#include "Character/AZPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Level/Lobby/AZPartyUI.h"
#include "GameFramework/PlayerState.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"

UAZSessionUIComponent::UAZSessionUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;

	ConstructorHelpers::FClassFinder<UAZLobbyUI> Session(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/SessionUI/WBP_LobbyLevelUI.WBP_LobbyLevelUI_C'"));
	if (Session.Succeeded())
	{
		SessionUIClass = Session.Class;
	}

	ConstructorHelpers::FClassFinder<UAZPartyUI> Party(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/SessionUI/WBP_PartyReadyUI.WBP_PartyReadyUI_C'"));
	if (Party.Succeeded())
	{
		PartyUIClass = Party.Class;
	}
}

void UAZSessionUIComponent::BeginPlay()
{
	Super::BeginPlay();	

	Owner = Cast<AAZPlayerController>(GetOwner());
	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Equals(TEXT("LobbyLevel_Party")))
	{
		ConstructUI();
	}
}

void UAZSessionUIComponent::ConstructUI()
{
	if (!Owner || !Owner->IsLocalController()) return;

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Equals(TEXT("LobbyLevel_Party")))
	{
		if (!PartyUI)
		{
			PartyUI = CreateWidget<UAZPartyUI>(Owner, PartyUIClass);
			if (!PartyUI) return;
		}

		PartyUI->AddToViewport(5);
		PartyUI->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		AAZPlayerCharacter* Character = Cast<AAZPlayerCharacter>(Owner->GetPawn());
		if (Character) Character->GetCharacterMovement()->GravityScale = 0;

		if (Character->HasAuthority())
		{
			PartyUI->UpdatePartyUI();
		}

		Owner->SetMouseMode(true);
		Owner->SetHUDVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		if (!SessionUI)
		{
			SessionUI = CreateWidget<UAZLobbyUI>(Owner, SessionUIClass);
		}

		if (SessionUI)
		{
			SessionUI->AddToViewport(10);
			SessionUI->SetVisibility(ESlateVisibility::Collapsed);
			Owner->SetMouseMode(false);
			SessionUI->OnNativeVisibilityChanged.AddUObject(this, &UAZSessionUIComponent::SetSessionUI);
		}
	}
}

void UAZSessionUIComponent::SetSessionUI(ESlateVisibility Visibility)
{
	if (Visibility == ESlateVisibility::SelfHitTestInvisible)
	{
		Owner->SetMouseMode(true);
		Owner->ChangeCameraViewTarget(Owner->GetPawn());
	}
	else
	{
		Owner->SetMouseMode(false);
	}
}

void UAZSessionUIComponent::OpenSessionUI()
{
	if (!bIsVisible) return;

	if (!SessionUI)
	{
		ConstructUI();
	}

	if (SessionUI->GetVisibility() == ESlateVisibility::SelfHitTestInvisible)
	{
		SessionUI->SetVisibility(ESlateVisibility::Collapsed);

		Owner->SetMouseMode(false);
	}
	else
	{
		SessionUI->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		Owner->SetMouseMode(true);
	}
}