// Fill out your copyright notice in the Description page of Project Settings.

#include "AZLobbyUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "System/AZSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "AZSessionItem.h"
#include "Components/WidgetSwitcher.h"
#include "System/Player/AZPlayerController.h"

void UAZLobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* const World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);

	OnNativeVisibilityChanged.AddUObject(this, &UAZLobbyUI::VisibilityChanged);
}

void UAZLobbyUI::OnClickCreateSessionBtn()
{
}

void UAZLobbyUI::OnClickFindSessionBtn()
{
}

FReply UAZLobbyUI::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	return  FReply::Handled();
}

void UAZLobbyUI::VisibilityChanged(ESlateVisibility InVisibility)
{
	if (InVisibility == ESlateVisibility::Collapsed)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			AAZPlayerController* PC = Cast<AAZPlayerController>(PlayerController);
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(false);

			PC->ChangeCameraViewTarget(PC->GetPawn());
			PC->SetHUDVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}