// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Level/Lobby/AZPartyUI.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Overlay.h"
#include "System/Subsystems/AZQuestManagerSubsystem.h"
#include "UI/Level/Lobby/AZQuestObjListItem.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "System/Player/AZPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanel.h"
#include "UI/Level/Lobby/AZPartyIcon.h"
#include "GameState/AZLobbyGameState.h"
#include "System/AZPlayerState.h"
#include "System/Subsystems/UAZCurrencySubsystem.h"
#include "AshZero.h"

void UAZPartyUI::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* const World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);

	Player1->SwitchIconState(EPartyIconState::Ready);

	if (GetOwningPlayer()->HasAuthority())
	{
		WidgetSwitcher->SetActiveWidgetIndex(0);
	}
	else
	{
		WidgetSwitcher->SetActiveWidgetIndex(1);
	}

	if (AAZLobbyGameState* GameState = GetWorld()->GetGameState<AAZLobbyGameState>())
	{
		GameState->OnPartyUpdated.AddUObject(this, &UAZPartyUI::UpdatePartyUI);
	}

	UAZQuestManagerSubsystem* QuestSystem = GetGameInstance()->GetSubsystem<UAZQuestManagerSubsystem>();
	if(QuestSystem)
	{ 
		const FAZQuest* Quest = QuestSystem->GetCurrentQuest();
		if (Quest)
		{
			QuestName->SetText(FText::FromName(Quest->QuestName));

			ObjectiveList->ClearChildren();
			float TotalCount = 0;
			float CurrentCount = 0;
			for (FAZQuestObjectiveData Objective : Quest->Objectives)
			{
				TotalCount += Objective.RequiredCount;
				CurrentCount += Objective.CurrentCount;
				UAZQuestObjListItem* ItemWidget = CreateWidget<UAZQuestObjListItem>(ObjectiveList, ObjectiveItemClass);
				ItemWidget->SetObjectiveData(Objective);
				ObjectiveList->AddChild(ItemWidget);
			}
			QuestBar->SetPercent(CurrentCount / TotalCount);
			ObjectiveList->AddChild(QuestBar);
		}
	}

	RoadOutBtn->OnClicked.AddDynamic(this, &UAZPartyUI::OnClickedRoadOut);
	ReadyBtn->OnClicked.AddDynamic(this, &UAZPartyUI::OnClickedReady);


	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UUAZCurrencySubsystem* CurrencySys = GI->GetSubsystem<UUAZCurrencySubsystem>())
		{
			OnCurrencyChanged(CurrencySys->GetMoney());
		}
	}
}

void UAZPartyUI::OnClickedRoadOut()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		AAZPlayerController* AZPC = Cast<AAZPlayerController>(PC);
		if (AZPC) AZPC->EnterRoadOut();
		SetVisibilityPartyUI(false);
	}
}

void UAZPartyUI::UpdatePartyUI()
{
	AAZLobbyGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AAZLobbyGameState>() : nullptr;
	APlayerController* PC = GetOwningPlayer();
	if (!GameState || !PC) return;

	AAZPlayerState* LocalPS = Cast<AAZPlayerState>(PC->PlayerState);
	if (!LocalPS) return;

	const bool bIsHost = PC->HasAuthority();
	AAZPlayerState* OtherPS = nullptr;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (PS == LocalPS) continue;
		if(AAZPlayerState* Other = Cast<AAZPlayerState>(PS))
		{
			OtherPS = Other;
			break;
		}
	}

	LocalOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	LocalName->SetText(FText::FromString(LocalPS->GetPlayerName()));

	if (OtherPS)
	{
		const bool bReadyToShow = bIsHost ? OtherPS->bIsReady : LocalPS->bIsReady;
		Player2->SwitchIconState(bReadyToShow ? EPartyIconState::Ready : EPartyIconState::NoneReady);

		if (bIsHost) GameStartBtn->SetIsEnabled(OtherPS->bIsReady);

		OtherOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		OtherName->SetText(FText::FromString(OtherPS->GetPlayerName()));
	}
	else
	{
		Player2->SwitchIconState(EPartyIconState::Empty);
		OtherOverlay->SetVisibility(ESlateVisibility::Collapsed);

		if (bIsHost) GameStartBtn->SetIsEnabled(true);
	}
}

void UAZPartyUI::OnClickedReady()
{
	AAZPlayerController* AZPC = Cast<AAZPlayerController>(GetOwningPlayer());
	if (!AZPC) return;
	AAZPlayerState* PS = AZPC->GetPlayerState<AAZPlayerState>();
	if (!PS) return;

	const bool bNewReady = !PS->bIsReady;
	Player2->SwitchIconState(bNewReady ? EPartyIconState::Ready : EPartyIconState::NoneReady);

	AZPC->SetReady_Server(bNewReady);
}

void UAZPartyUI::SetVisibilityPartyUI(bool bIsVisible)
{
	if (!bIsVisible)
	{
		QuestUI->SetVisibility(ESlateVisibility::Collapsed);
		RoadOutBtn->SetVisibility(ESlateVisibility::Collapsed);
		Player1->SetVisibility(ESlateVisibility::Collapsed);
		Player2->SetVisibility(ESlateVisibility::Collapsed);
		DestroySessionBtn->SetVisibility(ESlateVisibility::Collapsed);
		ExitSessionBtn->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		QuestUI->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		RoadOutBtn->SetVisibility(ESlateVisibility::Visible);
		Player1->SetVisibility(ESlateVisibility::HitTestInvisible);
		Player2->SetVisibility(ESlateVisibility::HitTestInvisible);
		DestroySessionBtn->SetVisibility(ESlateVisibility::Visible);
		ExitSessionBtn->SetVisibility(ESlateVisibility::Visible);
	}
}

void UAZPartyUI::OnCurrencyChanged(int32 NewAmount)
{
	if (GoldAmount == nullptr) return;
	GoldAmount->SetText(FText::AsNumber(NewAmount));
}