
#include "UI/DialogBox/AZDialogBoxUI.h"
#include "AZDialogButtonWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "System/Player/AZPlayerController.h"
#include "System/Player/Components/AZDialogUIComponent.h"
#include "System/Player/Components/AZSessionUIComponent.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/RichTextBlock.h"
#include "System/Subsystems/AZDialogSubsystem.h"
#include "System/Subsystems/AZQuestManagerSubsystem.h"
#include "System/AZSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Crafting/AZCraftingWidget.h"
#include "Character/AZPlayerCharacter.h"
#include "Weapon/AZWeapon.h"
#include "System/Subsystems/AZStoreSubsystem.h"

void UAZDialogBoxUI::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	OnNativeVisibilityChanged.AddUObject(this, &UAZDialogBoxUI::VisibilityChanged);

	if (GetGameInstance())
	{
		DialogSubsystem = GetGameInstance()->GetSubsystem<UAZDialogSubsystem>();
		QuestSubsystem = GetGameInstance()->GetSubsystem<UAZQuestManagerSubsystem>();
	}
	OwnerPC = Cast<AAZPlayerController>(GetOwningPlayer());
}

void UAZDialogBoxUI::SetActorDialog(FGameplayTag GiverTag, FEnableButtons EnableButtons)
{
	if (!DialogSubsystem.IsValid()) return;
	TalkDialog.Empty();
	DialogText->SetText(FText::FromString(TEXT("안녕하세요.")));

	if (TArray<FAZDialog>* Dialogs = DialogSubsystem->GetNPCDialogList(GiverTag))
	{
		for (const FAZDialog& Dialog : *Dialogs)
		{
			if (Dialog.DialogType == EDialogType::NormalTalk)
				TalkDialog.Add(Dialog);
			else if (Dialog.DialogType == EDialogType::Greetings)
				DialogText->SetText(Dialog.DialogRows[0].Line);
		}
	}

	NPCTag = GiverTag;
	NameText = DialogSubsystem->GetTagText(GiverTag);
	SpeakerName->SetText(NameText);

	ButtonWrapper->ClearChildren();
	CreateFeatureButton(FText::FromString(TEXT("대화")), &UAZDialogBoxUI::OnConversationButtonClicked);

	if (QuestSubsystem.IsValid())
	{
		FAZQuest* Quest = QuestSubsystem->GetAvailableQuest(NPCTag);
		if (Quest)
			CreateFeatureButton(FText::FromString(TEXT("퀘스트")), &UAZDialogBoxUI::OnQuestButtonClicked);
	}

	if(EnableButtons.bCrafting)
		CreateFeatureButton(FText::FromString(TEXT("제작")), &UAZDialogBoxUI::OnCraftingButtonClicked);
	if (EnableButtons.bTrade)
		CreateFeatureButton(FText::FromString(TEXT("상점")), &UAZDialogBoxUI::OnTradeButtonClicked);
	if (EnableButtons.bEnhance)
		CreateFeatureButton(FText::FromString(TEXT("강화")), &UAZDialogBoxUI::OnEnhanceButtonClicked);

	UAZSessionSubsystem* SessionSystem = GetGameInstance()->GetSubsystem<UAZSessionSubsystem>();
	if (SessionSystem && EnableButtons.bSession)
		CreateFeatureButton(FText::FromString(TEXT("진입")), &UAZDialogBoxUI::OnSessionButtonClicked);
}

void UAZDialogBoxUI::CreateFeatureButton(EDialogButtonType ButtonType, FText ButtonText)
{
	UAZDialogButtonWidget* ButtonWidget = CreateWidget<UAZDialogButtonWidget>(ButtonWrapper, ButtonClass);
	if (!ButtonWidget) return;

	UVerticalBoxSlot* BoxSlot = ButtonWrapper->AddChildToVerticalBox(ButtonWidget);
	ButtonWidget->ButtonText->SetText(ButtonText);
	BoxSlot->SetPadding(FMargin(0.f, 10.f, 0.f, 10.f));

	switch (ButtonType)
	{
	case EDialogButtonType::Talk:
		ButtonWidget->OnDialogButtonClicked.AddUObject(this, &UAZDialogBoxUI::OnConversationButtonClicked);
		break;
	case EDialogButtonType::Quest:
		ButtonWidget->OnDialogButtonClicked.AddUObject(this, &UAZDialogBoxUI::OnQuestButtonClicked);
		break;
	case EDialogButtonType::QuestAccept:
		ButtonWidget->OnDialogButtonClicked.AddUObject(this, &UAZDialogBoxUI::OnQuestAcceptButtonClicked);
		break;
	case EDialogButtonType::QuestDecline:
		ButtonWidget->OnDialogButtonClicked.AddUObject(this, &UAZDialogBoxUI::OnQuestDeclineButtonClicked);
		break;
	case EDialogButtonType::OpenShop:
		ButtonWidget->OnDialogButtonClicked.AddUObject(this, &UAZDialogBoxUI::OnTradeButtonClicked);
		break;
	case EDialogButtonType::OpenCrafting:
		ButtonWidget->OnDialogButtonClicked.AddUObject(this, &UAZDialogBoxUI::OnCraftingButtonClicked);
		break;
	case EDialogButtonType::OpenEnhancement:
		ButtonWidget->OnDialogButtonClicked.AddUObject(this, &UAZDialogBoxUI::OnEnhanceButtonClicked);
		break;
	case EDialogButtonType::OpenSession:
		ButtonWidget->OnDialogButtonClicked.AddUObject(this, &UAZDialogBoxUI::OnSessionButtonClicked);
		break;
	default:
		break;
	}
}

void UAZDialogBoxUI::CreateFeatureButton(FText ButtonText, FButtonHandlerFunc ButtonHandler)
{
	UAZDialogButtonWidget* ButtonWidget = CreateWidget<UAZDialogButtonWidget>(ButtonWrapper, ButtonClass);
	if (!ButtonWidget) return;

	UVerticalBoxSlot* BoxSlot = ButtonWrapper->AddChildToVerticalBox(ButtonWidget);
	ButtonWidget->ButtonText->SetText(ButtonText);
	BoxSlot->SetPadding(FMargin(0.f, 10.f, 0.f, 10.f));

	ButtonWidget->OnDialogButtonClicked.AddUObject(this, ButtonHandler);
}

void UAZDialogBoxUI::OnConversationButtonClicked(UButton* ClickedButton)
{
	if (TalkDialog.Num() == 0) return;

	int32 DialogIdx = FMath::RandRange(0, TalkDialog.Num() - 1);

	CurrentDialog = TalkDialog[DialogIdx];
	CurrentLogIndex = 0;
	if (!CurrentDialog.IsSet()) return;

	SetButtonVisibility(false);
	GoToNextLine();
}

void UAZDialogBoxUI::OnCraftingButtonClicked(UButton* ClickedButton)
{
	if (OwnerPC.IsValid() && OwnerPC->CraftingWidget)
	{
		OwnerPC->SetInputMode(FInputModeUIOnly());
		OwnerPC->CraftingWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		OwnerPC->ChangeCameraViewTarget(OwnerPC->GetPawn());
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAZDialogBoxUI::OnQuestButtonClicked(UButton* ClickedButton)
{
	if (!QuestSubsystem.IsValid() || !DialogSubsystem.IsValid()) return;
	FAZQuest* AvailableQuest = QuestSubsystem->GetAvailableQuest(NPCTag);
	if (!AvailableQuest) return;

	const FAZQuest* CurrentQuest = QuestSubsystem->GetCurrentQuest();
	const bool bIsCurrentQuest = (CurrentQuest != nullptr) && (AvailableQuest->QuestTag == CurrentQuest->QuestTag);
	const FGameplayTag QuestTag = bIsCurrentQuest ? CurrentQuest->QuestTag : AvailableQuest->QuestTag;

	TArray<EDialogType, TInlineAllocator<2>> TypesToTry;
	if (bIsCurrentQuest)
	{
		TypesToTry.Add(CurrentQuest->bIsComplete ? EDialogType::CompletedQuest : EDialogType::InProgressQuest);
	}
	else
	{
		TypesToTry.Add(EDialogType::BeforeAcceptQuest);
		TypesToTry.Add(EDialogType::AcceptQuest);
	}

	FAZDialog Dialog;
	for (EDialogType Type : TypesToTry)
	{
		Dialog = DialogSubsystem->GetQuestDialog(NPCTag, QuestTag, Type);
		if (Dialog.DialogRows.IsEmpty()) continue;

		CurrentDialog = Dialog;
		CurrentLogIndex = 0;
		SetButtonVisibility(false);
		GoToNextLine();
		return;
	}
}

void UAZDialogBoxUI::OnQuestAcceptButtonClicked(UButton* ClickedButton)
{
	UAZDialogSubsystem* DialogSystem = GetGameInstance()->GetSubsystem<UAZDialogSubsystem>();
	if (!DialogSystem) return;

	FAZDialog Dialog = DialogSystem->GetQuestDialog(NPCTag, CurrentDialog.GetValue().ContextTag, EDialogType::AcceptQuest);
	if (Dialog.DialogRows.Num() <= 0)
	{
		ExitDialogUI();
		return;
	}
	CurrentDialog = Dialog;
	CurrentLogIndex = 0;
	if (!CurrentDialog.IsSet()) return;

	SetButtonVisibility(false);
	GoToNextLine();
}

void UAZDialogBoxUI::OnQuestDeclineButtonClicked(UButton* ClickedButton)
{
	UAZQuestManagerSubsystem* QuestSystem = GetGameInstance()->GetSubsystem<UAZQuestManagerSubsystem>();
	if (!QuestSystem) return;

	UAZDialogSubsystem* DialogSystem = GetGameInstance()->GetSubsystem<UAZDialogSubsystem>();
	if (!DialogSystem) return;

	FAZDialog Dialog = DialogSystem->GetQuestDialog(NPCTag, CurrentDialog.GetValue().ContextTag, EDialogType::DeclineQuest);

	if (Dialog.DialogRows.Num() <= 0)
	{
		ExitDialogUI();
		return;
	}

	CurrentDialog = Dialog;
	CurrentLogIndex = 0;
	if (!CurrentDialog.IsSet()) return;

	SetButtonVisibility(false);
	GoToNextLine();
}

void UAZDialogBoxUI::OnTradeButtonClicked(UButton* ClickedButton)
{
	if (UWorld* World = GetWorld())
	{
		UAZStoreSubsystem* StoreSubsystem = World->GetSubsystem<UAZStoreSubsystem>();
		if (StoreSubsystem)
		{
			StoreSubsystem->ShowWidget();
		}
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		AAZPlayerController* PC = Cast<AAZPlayerController>(PlayerController);
		if (PC)
		{
			PC->ChangeCameraViewTarget(PC->GetPawn());
			SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UAZDialogBoxUI::OnEnhanceButtonClicked(UButton* ClickedButton)
{
}

void UAZDialogBoxUI::OnSessionButtonClicked(UButton* ClickedButton)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		AAZPlayerController* PC = Cast<AAZPlayerController>(PlayerController);
		if (PC) PC->SessionUIComp->OpenSessionUI();

		PC->ChangeCameraViewTarget(PC->GetPawn());
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

FReply UAZDialogBoxUI::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::F && !CurrentDialog.IsSet())
	{
		ExitDialogUI();
	}

	return  FReply::Handled();
}

FReply UAZDialogBoxUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (!CurrentDialog.IsSet()) return FReply::Handled();
		if (CurrentDialog->DialogRows[CurrentLogIndex].bHasChoices) return FReply::Handled();
		CurrentLogIndex++;
		GoToNextLine();
	}

	return FReply::Handled();
}

void UAZDialogBoxUI::GoToNextLine()
{
	if (CurrentLogIndex >= CurrentDialog->DialogRows.Num())
	{
		EndDialogConversation();
		return;
	}

	const auto& CurrentRow = CurrentDialog->DialogRows[CurrentLogIndex];
	if (CurrentRow.Speaker == ESpeakerType::NPC)
	{
		SpeakerName->SetText(NameText);
	}
	else if (CurrentRow.Speaker == ESpeakerType::Player)
	{
		SpeakerName->SetText(FText::FromString(TEXT("엘바시아")));
	}
	else
	{
		if (DialogSubsystem.IsValid() && CurrentRow.CustomSpeakerTag.IsValid())
		{
			SpeakerName->SetText(DialogSubsystem->GetTagText(CurrentRow.CustomSpeakerTag));
		}
		else
		{
			SpeakerName->SetText(FText::GetEmpty());
		}
	}

	DialogText->SetText(CurrentRow.Line);

	if (CurrentRow.bHasChoices)
	{
		for (const FAZDialogChoice& Choice : CurrentRow.Choices)
		{
			CreateFeatureButton(Choice.ButtonType, Choice.ButtonText);
		}
	}
}

void UAZDialogBoxUI::VisibilityChanged(ESlateVisibility InVisibility)
{
	if (InVisibility == ESlateVisibility::Collapsed)
	{
		TArray<UWidget*> Buttons = ButtonWrapper->GetAllChildren();
		for (UWidget* Button : Buttons)
		{
			UAZDialogButtonWidget* Widget = Cast<UAZDialogButtonWidget>(Button);
			if (Widget) Widget->SelectButton->OnClicked.Clear();
		}
		ButtonWrapper->ClearChildren();

		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			APawn* Pawn = PlayerController->GetPawn();
			if (!Pawn) return;
			AAZPlayerController* PC = Cast<AAZPlayerController>(PlayerController);
			PC->ChangeCameraViewTarget(Pawn);
			PC->SetHUDVisibility(ESlateVisibility::HitTestInvisible);

			if (AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(Pawn))
			{
				Player->SetActorHiddenInGame(false);
			}
		}
	}
	else
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (!PlayerController) return;
		APawn* Pawn = PlayerController->GetPawn();
		if (!Pawn) return;

		if (AAZPlayerCharacter* Player = Cast<AAZPlayerCharacter>(Pawn))
		{
			Player->SetActorHiddenInGame(true);
		}
	}
}

void UAZDialogBoxUI::ExitDialogUI()
{
	CurrentDialog.Reset();
	SetVisibility(ESlateVisibility::Collapsed);

	if (OwnerPC.IsValid())
	{
		OwnerPC->SetInputMode(FInputModeGameOnly());
		OwnerPC->SetShowMouseCursor(false);
	}
}

void UAZDialogBoxUI::SetButtonVisibility(bool bVisible)
{
	TArray<UWidget*> Children = ButtonWrapper->GetAllChildren();
	if (bVisible)
	{
		for (UWidget* Widget : Children)
		{
			Widget->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		for (UWidget* Widget : Children)
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UAZDialogBoxUI::EndDialogConversation()
{
	CurrentLogIndex = 0;
	EDialogType Type = CurrentDialog.GetValue().DialogType;

	if (Type == EDialogType::NormalTalk)
	{
		SetButtonVisibility(true);
		CurrentDialog.Reset();
		return;
	}

	if (QuestSubsystem.IsValid())
	{
		if (Type == EDialogType::CompletedQuest)
		{
			QuestSubsystem->SpawnReward();
			QuestSubsystem->CompleteCurrentQuest();
		}
		else if (Type == EDialogType::AcceptQuest)
		{
			QuestSubsystem->AcceptQuest(CurrentDialog.GetValue().ContextTag);
		}
	}

	ExitDialogUI();
}