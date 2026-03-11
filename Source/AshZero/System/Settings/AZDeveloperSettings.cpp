// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Settings/AZDeveloperSettings.h"

FName UAZDeveloperSettings::GetCategoryName() const
{
	return "Data Settings";
}
#if WITH_EDITOR
FText UAZDeveloperSettings::GetSectionText() const
{
	return FText::FromString("Custom Data");
}

FText UAZDeveloperSettings::GetSectionDescription() const
{
	return FText::FromString("Manage data tables, data assets, and classes to be used in the game.");
}
#endif