// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Level/Lobby/AZPartyIcon.h"
#include "Components/WidgetSwitcher.h"

void UAZPartyIcon::SwitchIconState(EPartyIconState IconState)
{
	IconSwitcher->SetActiveWidgetIndex(static_cast<int32>(IconState));
}