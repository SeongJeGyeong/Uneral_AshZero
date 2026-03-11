// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Player/Components/AZDialogUIComponent.h"
#include "UI/DialogBox/AZDialogBoxUI.h"
#include "System/Player/AZPlayerController.h"
#include "Character/AZPlayerCharacter.h"
#include "UI/HUD/AZStatusBarWidget.h"
#include "System/Player/Components/AZInteractionUIComponent.h"

// Sets default values for this component's properties
UAZDialogUIComponent::UAZDialogUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;

	ConstructorHelpers::FClassFinder<UAZDialogBoxUI> DialogUI(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/DialogBox/WBP_DialogBox.WBP_DialogBox_C'"));
	if (DialogUI.Succeeded())
	{
		DialogBoxWidgetClass = DialogUI.Class;
	}
}

// Called when the game starts
void UAZDialogUIComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<AAZPlayerController>(GetOwner());
}

void UAZDialogUIComponent::ConstructUI()
{
	DialogBoxWidget = CreateWidget<UAZDialogBoxUI>(Owner, DialogBoxWidgetClass);
	if (DialogBoxWidget)
	{
		DialogBoxWidget->AddToViewport(1);
		//DialogBoxWidget->SetVisibility(ESlateVisibility::Collapsed);
		DialogBoxWidget->OnNativeVisibilityChanged.AddUObject(this, &UAZDialogUIComponent::SetDialogBox);
		DialogBoxWidget->OnNativeVisibilityChanged.AddUObject(Owner->InteractionUIComp, &UAZInteractionUIComponent::SetVisibleState);
	}
}

void UAZDialogUIComponent::SetDialogBox(ESlateVisibility Visibility)
{
	if (Visibility == ESlateVisibility::Visible)
	{
		OriginViewTarget = Owner->GetViewTarget();
		DialogBoxWidget->SetKeyboardFocus();

		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Owner->SetInputMode(InputMode);
		Owner->bShowMouseCursor = true;
	}
}

void UAZDialogUIComponent::OpenDialogBox(FGameplayTag GiverTag, FEnableButtons EnableButtons)
{
	if (!DialogBoxWidget)
	{
		ConstructUI();
		SetDialogBox(ESlateVisibility::Visible);
		Owner->InteractionUIComp->SetVisibleState(ESlateVisibility::Visible);
	}

	if(DialogBoxWidget->Visibility == ESlateVisibility::Collapsed)
	{
		DialogBoxWidget->SetVisibility(ESlateVisibility::Visible);
	}
	DialogBoxWidget->SetActorDialog(GiverTag, EnableButtons);

	Owner->SetHUDVisibility(ESlateVisibility::Collapsed);
}